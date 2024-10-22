//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#pragma once


#include <string>
#include <vector>
//#include <GLES3/gl32.h>

#define GLCORE_BEGIN(ns) namespace ns {
#define GLCORE_END(ns)   }


GLCORE_BEGIN(glutil)


/**
 * @brief Buffer structure
 */
struct Buffer {
    unsigned int handle = 0;
    int size;
    unsigned int target;

    Buffer() {}
    Buffer(unsigned int handle, int size, unsigned int target) : handle(handle), size(size), target(target) {}
    operator unsigned int() const {
        return handle;
    }
    operator unsigned int*() {
        return &handle;
    }
    void bindBase(unsigned int binding) const;
    void subData(const void* data) const;
    /**
     * @brief Create a Buffer
     */
    static Buffer create(unsigned int target, int size, unsigned int usage);
    /**
     * @brief Delete texture2D
     *
     * `buf` should be created with Buffer::create()
     */
    static void destroy(Buffer& buf);
};

/**
 * @brief Create shader
 *
 * Return 0 when failed to create or compile shader.
 * Remember to `glDeleteShader()` after attached to program.
 *
 * @return Shader handle
 */
unsigned int createShader(unsigned int shader_type, const std::string& source);

/**
 * @brief Create compute shader program
 *
 * Return 0 when failed to create or link shader
 *
 * @return Shader program handle
 */
unsigned int createCompProg(const std::string& source, unsigned int& shader);

unsigned int createGraphicsProg( const std::string& strVertexShader, const std::string& strFragmentShader);

unsigned int getFormatFromSurfaceFormat(unsigned int format);

unsigned int getTypeFromSurfaceFormat(unsigned int format);

GLCORE_END(glcore)
