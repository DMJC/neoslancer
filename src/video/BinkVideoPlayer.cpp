#include "neoslancer/video/BinkVideoPlayer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <cstdio>

namespace neoslancer {

BinkVideoPlayer::~BinkVideoPlayer() {
    close();
}

bool BinkVideoPlayer::open(const std::string& path) {
    close();

    if (avformat_open_input(&m_formatContext, path.c_str(), nullptr, nullptr) != 0) {
        std::fprintf(stderr, "neoslancer: failed to open video '%s'\n", path.c_str());
        return false;
    }
    if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
        close();
        return false;
    }

    const AVCodec* decoder = nullptr;
    m_videoStreamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (m_videoStreamIndex < 0 || !decoder) {
        std::fprintf(stderr, "neoslancer: no decodable video stream in '%s'\n", path.c_str());
        close();
        return false;
    }

    m_codecContext = avcodec_alloc_context3(decoder);
    if (!m_codecContext) {
        close();
        return false;
    }
    const AVStream* stream = m_formatContext->streams[m_videoStreamIndex];
    if (avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0) {
        close();
        return false;
    }
    if (avcodec_open2(m_codecContext, decoder, nullptr) < 0) {
        close();
        return false;
    }

    m_width = m_codecContext->width;
    m_height = m_codecContext->height;

    if (stream->avg_frame_rate.num > 0 && stream->avg_frame_rate.den > 0) {
        m_frameDurationSeconds =
            static_cast<double>(stream->avg_frame_rate.den) / static_cast<double>(stream->avg_frame_rate.num);
    }

    m_frame = av_frame_alloc();
    m_rgbFrame = av_frame_alloc();
    m_packet = av_packet_alloc();
    if (!m_frame || !m_rgbFrame || !m_packet) {
        close();
        return false;
    }

    m_rgbBuffer.resize(static_cast<size_t>(m_width) * m_height * 3);
    av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize, m_rgbBuffer.data(), AV_PIX_FMT_RGB24, m_width,
                          m_height, 1);

    m_swsContext = sws_getContext(m_width, m_height, m_codecContext->pix_fmt, m_width, m_height, AV_PIX_FMT_RGB24,
                                   SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!m_swsContext) {
        close();
        return false;
    }

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_accumulatorSeconds = 0.0;
    m_finished = false;
    decodeNextFrame(); // show the first frame immediately, don't wait a tick
    return true;
}

void BinkVideoPlayer::close() {
    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    if (m_rgbFrame) {
        av_frame_free(&m_rgbFrame);
    }
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
    }
    m_videoStreamIndex = -1;
    m_width = 0;
    m_height = 0;
    m_frameDurationSeconds = 1.0 / 15.0;
    m_accumulatorSeconds = 0.0;
    m_finished = false;
}

bool BinkVideoPlayer::decodeNextFrame() {
    if (!m_formatContext || m_finished) {
        return false;
    }

    while (true) {
        const int receiveResult = avcodec_receive_frame(m_codecContext, m_frame);
        if (receiveResult == 0) {
            uploadFrame();
            return true;
        }
        if (receiveResult != AVERROR(EAGAIN) && receiveResult != AVERROR_EOF) {
            return false; // real decode error
        }

        const int readResult = av_read_frame(m_formatContext, m_packet);
        if (readResult < 0) {
            // End of stream: hold on the last decoded/uploaded frame rather
            // than looping (see update()'s doc comment).
            m_finished = true;
            return false;
        }

        if (m_packet->stream_index == m_videoStreamIndex) {
            avcodec_send_packet(m_codecContext, m_packet);
        }
        av_packet_unref(m_packet);
    }
}

void BinkVideoPlayer::uploadFrame() {
    sws_scale(m_swsContext, m_frame->data, m_frame->linesize, 0, m_height, m_rgbFrame->data, m_rgbFrame->linesize);

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_rgbBuffer.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BinkVideoPlayer::update(float deltaSeconds) {
    if (!isOpen()) {
        return;
    }
    m_accumulatorSeconds += deltaSeconds;
    while (m_accumulatorSeconds >= m_frameDurationSeconds) {
        m_accumulatorSeconds -= m_frameDurationSeconds;
        if (!decodeNextFrame()) {
            break;
        }
    }
}

} // namespace neoslancer
