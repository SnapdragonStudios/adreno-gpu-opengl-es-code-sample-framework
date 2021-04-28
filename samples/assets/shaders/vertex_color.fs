// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#version 300 es

in vec4 vertexColor;
out vec4 g_FragColor;

void main()
{
    g_FragColor = vertexColor;
}