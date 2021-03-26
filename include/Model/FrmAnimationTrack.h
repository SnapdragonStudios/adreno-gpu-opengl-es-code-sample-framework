// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_ANIMATION_TRACK_H
#define ADRENO_ANIMATION_TRACK_H

#include "FrmNamedId.h"
#include "FrmPlatform.h"
#include "FrmTransform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct AnimationTrack
    {
        NamedId    Id;
        INT32      NumKeyframes;
        Transform* Keyframes;

         AnimationTrack();
        ~AnimationTrack();

        void    SetName         ( const char* name );
        void    ResizeKeyframes ( int num_keyframes );

        private:
            void DeleteKeyframes();
    };
}

//-----------------------------------------------------------------------------

#endif