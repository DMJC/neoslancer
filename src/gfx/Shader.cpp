#include "neoslancer/gfx/Shader.h"

#include <cstdio>
#include <vector>

namespace neoslancer {

namespace {

GLuint compileStage(GLenum stage, const char* source) {
    const GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(logLength) + 1);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::fprintf(stderr, "shader compile error: %s\n", log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

Shader::~Shader() {
    destroy();
}

bool Shader::compile(const char* vertexSource, const char* fragmentSource) {
    destroy();

    const GLuint vertex = compileStage(GL_VERTEX_SHADER, vertexSource);
    if (vertex == 0) {
        return false;
    }
    const GLuint fragment = compileStage(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success = GL_FALSE;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(logLength) + 1);
        glGetProgramInfoLog(m_program, logLength, nullptr, log.data());
        std::fprintf(stderr, "shader link error: %s\n", log.data());
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    return true;
}

void Shader::destroy() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

void Shader::use() const {
    glUseProgram(m_program);
}

GLint Shader::uniformLocation(const char* name) const {
    return glGetUniformLocation(m_program, name);
}

} // namespace neoslancer
