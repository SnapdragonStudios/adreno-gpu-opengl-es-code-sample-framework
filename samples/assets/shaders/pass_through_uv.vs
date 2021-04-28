// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#version 300 es

in  vec4 inVertex;
in  vec2 inUV;

out vec2 uv;

void main()
{
    gl_Position  = inVertex;
    uv = inUV;
}