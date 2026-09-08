#pragma once

#include <GL/glew.h>

#include <cstdint>
#include <string>
#include <vector>

struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
struct AVFrame;
struct AVPacket;

namespace neoslancer {

// Plays the original engine's `.bik` (Bink 1.x) assets. The original
// pipeline (../StarLancer/reversing docs Pass 37) streams these directly
// from an open .hog archive handle via RAD Game Tools' proprietary Bink
// SDK (_BinkOpen/_BinkDoFrame/_BinkCopyToBuffer/_BinkClose) - that SDK
// isn't available here, and decoding Bink's actual codec was never a goal
// of this port. Instead this plays the same real .bik files (they exist
// as loose files under the game data root, e.g. cd1/b2iloop.bik) through
// FFmpeg's own from-scratch (independently reverse-engineered, LGPL)
// Bink video decoder - confirmed working against real Star Lancer assets
// in this session (ffprobe/ffmpeg correctly identify and decode them as
// yuv420p 640x480 15fps "binkvideo").
//
// Decodes on the calling thread, one frame at a time, paced by the
// stream's own frame rate rather than real Bink SDK timing/audio-sync
// semantics - close enough for menu-adjacent playback, not frame-exact.
// Video only: some real .bik files (the logo/splash intro sequence) do
// carry an audio track, but this player only opens the video stream -
// no audio output is implemented.
class BinkVideoPlayer {
public:
    BinkVideoPlayer() = default;
    ~BinkVideoPlayer();

    BinkVideoPlayer(const BinkVideoPlayer&) = delete;
    BinkVideoPlayer& operator=(const BinkVideoPlayer&) = delete;

    bool open(const std::string& path);
    void close();
    bool isOpen() const { return m_formatContext != nullptr; }

    // Advances playback by deltaSeconds, decoding as many frames as have
    // elapsed and uploading the latest one to the GL texture. Plays once
    // and holds on the final decoded frame at end-of-stream - matches the
    // original's one-shot room-transition clips; this port doesn't
    // distinguish those from the handful of genuinely-looping idle shots
    // (only entry's "b2iloop.bik" has "loop" in its own name).
    void update(float deltaSeconds);

    bool isFinished() const { return m_finished; }

    GLuint texture() const { return m_texture; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    bool decodeNextFrame();
    void uploadFrame();

    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    SwsContext* m_swsContext = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_rgbFrame = nullptr;
    AVPacket* m_packet = nullptr;
    int m_videoStreamIndex = -1;

    std::vector<uint8_t> m_rgbBuffer;
    GLuint m_texture = 0;
    int m_width = 0;
    int m_height = 0;

    double m_frameDurationSeconds = 1.0 / 15.0;
    double m_accumulatorSeconds = 0.0;
    bool m_finished = false;
};

} // namespace neoslancer
