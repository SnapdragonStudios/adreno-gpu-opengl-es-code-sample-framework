// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#version 300 es

in  vec4 inVertex;
in  vec4 inColor;

out vec4 vertexColor;

void main()
{
    gl_Position  = inVertex;
    vertexColor = inColor;
}