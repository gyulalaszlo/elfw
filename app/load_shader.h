#pragma once

#include <GL/glew.h>

namespace glhelpers {
    GLuint LoadShaders(const char* logName, const char* vertexShaderCode, const char* fragmentShaderCode);
}
