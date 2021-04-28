// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#version 300 es

uniform sampler2D g_Texture;

in vec2 uv;
out vec4 g_FragColor;

void main()
{
    const float size = 1.3;
    const float sharpness = 4.0;

    vec4 tex = texture(g_Texture, uv);

    float l = length(uv - vec2(0.5,0.5))*size;
    float res = pow(l, sharpness);

    g_FragColor = vec4(tex.xyz*(1.0-res), 1.0);
}