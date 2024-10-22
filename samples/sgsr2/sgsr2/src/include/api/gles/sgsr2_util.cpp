//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "api/gles/sgsr2_util.h"
#include <cassert>
#include <GLES3/gl32.h>
GLCORE_BEGIN(glutil)

void Buffer::subData(const void* data) const {
    glBindBuffer(target, handle);
    glBufferSubData(target, 0, size, data);
    glBindBuffer(target, 0);
}

void Buffer::bindBase(unsigned int binding) const {
    glBindBufferBase(target, binding, handle);
}

Buffer Buffer::create(GLenum target, GLsizei size, GLenum usage) {
    Buffer buf{};
    glGenBuffers(1, buf);
    glBindBuffer(target, buf);
    glBufferData(target, size, nullptr, usage);
    glBindBuffer(target, 0);
    buf.size = size;
    buf.target = target;
    return buf;
}

void Buffer::destroy(Buffer& buf) {
    glDeleteBuffers(1, buf);
    buf.handle = 0;
}

static bool checkCompiler(GLuint shader) {
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled) {
        return true;
    }

    GLint info_log_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
    if (info_log_len > 0) {
        std::vector<char> info_log(info_log_len);
        glGetShaderInfoLog(shader, info_log_len, NULL, info_log.data());
    }
    return false;
}

GLuint createShader(GLenum shader_type, const std::string& source) {
    // Create shader with glsl source code
    GLuint shader = glCreateShader(shader_type);

    // Compile shader
    auto data = source.c_str();
    glShaderSource(shader, 1, &data, NULL);
    glCompileShader(shader);
    if (checkCompiler(shader)) {
        return shader;
    }

    // Must delete created shader as failed to compile
    glDeleteShader(shader);
    return 0;
}

static bool checkLinker(GLuint program) {
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked) {
        return true;
    }

    GLint info_log_len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_len);
    if (info_log_len > 0) {
        std::vector<char> info_log(info_log_len);
        glGetProgramInfoLog(program, info_log_len, NULL, info_log.data());
    }
    return false;
}

GLuint createCompProg(const std::string& source, GLuint& shader) {
    shader = createShader(GL_COMPUTE_SHADER, source);
    if (!shader) {
        return 0;
    }

    // Create program for shader
    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, shader);
        glLinkProgram(program);
        if (!checkLinker(program)) {
            glDeleteProgram(program);
            program = 0;
        }
    }

    // Must delete created shader
    glDeleteShader(shader);
    return program;
}

GLuint createGraphicsProg( const std::string& strVertexShader,
                              const std::string& strFragmentShader)
{
    GLuint hVertexShader = createShader(GL_VERTEX_SHADER, strVertexShader);
    if (!hVertexShader) {
        return 0;
    }

    GLuint hFragmentShader = createShader(GL_FRAGMENT_SHADER, strFragmentShader);
    if (!hFragmentShader) {
        return 0;
    }

    // Attach the individual shaders to the common shader program
    GLuint hShaderProgram  = glCreateProgram();
    glAttachShader( hShaderProgram, hVertexShader );
    glAttachShader( hShaderProgram, hFragmentShader );
    glLinkProgram(hShaderProgram);
    if (!checkLinker(hShaderProgram)) {
        glDeleteProgram(hShaderProgram);
        hShaderProgram = 0;
    }

    // Must delete created shader
    glDeleteShader(hVertexShader);
    glDeleteShader(hFragmentShader);
    return hShaderProgram;
}

GLenum getFormatFromSurfaceFormat(GLenum format){
    switch (format){
        case GL_RGBA32F:
        case GL_RGBA16F:
        case GL_RGBA8:
        case GL_RGB10_A2:
            return GL_RGBA;
        case GL_R11F_G11F_B10F:
            return GL_RGB;
        case GL_R32F:
            return GL_RED;
        case GL_R32UI:
            return GL_RED_INTEGER;
        case GL_DEPTH24_STENCIL8:
            return GL_DEPTH_STENCIL;
        default:
            return 0;
    }
}

GLenum getTypeFromSurfaceFormat(GLenum format){
    switch (format){
        case GL_RGBA32F:
        case GL_R32F:
            return GL_FLOAT;
        case GL_RGBA16F:
            return GL_HALF_FLOAT;
        case GL_RGBA8:
            return GL_UNSIGNED_BYTE;
        case GL_R32UI:
            return GL_UNSIGNED_INT;
        case GL_RGB10_A2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case GL_R11F_G11F_B10F:
            return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case GL_DEPTH24_STENCIL8:
            return GL_UNSIGNED_INT_24_8;
        default:
            return 0;
    }

}

GLCORE_END(glcore)
