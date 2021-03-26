// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmAnimation.h"

//-----------------------------------------------------------------------------

Adreno::Animation::Animation()
: NumFrames( 0 )
, NumTracks( 0 )
, Tracks( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::Animation::~Animation()
{
    DeleteTracks();
}

//-----------------------------------------------------------------------------

void Adreno::Animation::ResizeTracks( int num_tracks )
{
    DeleteTracks();
    
    NumTracks = num_tracks;
    
    if( num_tracks > 0 )
    {
        Tracks = new Adreno::AnimationTrack[ num_tracks ];
    }
}

//-----------------------------------------------------------------------------

void Adreno::Animation::DeleteTracks()
{
    if( Tracks )
    {
        delete[] Tracks;
        Tracks = NULL;
    }
}
