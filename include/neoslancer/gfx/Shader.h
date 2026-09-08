#pragma once

#include <GL/glew.h>

#include <string>

namespace neoslancer {

// Minimal GLSL program wrapper - compiles a vertex+fragment pair and owns
// the resulting program object.
class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool compile(const char* vertexSource, const char* fragmentSource);
    void destroy();

    void use() const;
    GLint uniformLocation(const char* name) const;

    GLuint id() const { return m_program; }

private:
    GLuint m_program = 0;
};

} // namespace neoslancer
