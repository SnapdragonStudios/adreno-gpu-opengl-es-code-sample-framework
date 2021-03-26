// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_NOISE_H_
#define _FRM_NOISE_H_

#include "FrmPlatform.h"
#include "FrmMath.h"


#define B 0x100
#define BM 0xff

#define N 0x1000

//--------------------------------------------------------------------------------------
// Name: class CFrmPerlin
// Desc: Class that implements improved perlin noise
//--------------------------------------------------------------------------------------
class CFrmPerlin
{
public:
    CFrmPerlin();

    FLOAT32 Noise1D( const FLOAT32 x );
    FLOAT32 Noise2D( const FLOAT32 x, const FLOAT32 y );
    FLOAT32 Noise3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z );

    FLOAT32 Turbulence1D( const FLOAT32 x,
                          FLOAT32 fFrequency, UINT32 nNumOctaves,
                          FLOAT32 fAmplitude=1.0f, FLOAT32 fPersistence=0.5f );
    FLOAT32 Turbulence2D( const FLOAT32 x, const FLOAT32 y,
                          FLOAT32 fFrequency, UINT32 nNumOctaves,
                          FLOAT32 fAmplitude=1.0f, FLOAT32 fPersistence=0.5f );
    FLOAT32 Turbulence3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z,
                          FLOAT32 fFrequency, UINT32 nNumOctaves,
                          FLOAT32 fAmplitude=1.0f, FLOAT32 fPersistence=0.5f );

    FLOAT32 TileableNoise1D( const FLOAT32 x, const FLOAT32 w );
    FLOAT32 TileableNoise2D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 w, const FLOAT32 h );
    FLOAT32 TileableNoise3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, const FLOAT32 w, const FLOAT32 h, const FLOAT32 d );

    FLOAT32 TileableTurbulence1D( const FLOAT32 x, 
                                  const FLOAT32 w, 
                                  FLOAT32 fFrequency, UINT32 nNumOctaves,
                                  FLOAT32 fAmplitude=1.0f, FLOAT32 fPersistence=0.5f );
    FLOAT32 TileableTurbulence2D( const FLOAT32 x, const FLOAT32 y, 
                                  const FLOAT32 w, const FLOAT32 h, 
                                  FLOAT32 fFrequency, UINT32 nNumOctaves,
                                  FLOAT32 fAmplitude=1.0f, FLOAT32 fPersistence=0.5f );
    FLOAT32 TileableTurbulence3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, 
                                  const FLOAT32 w, const FLOAT32 h, const FLOAT32 d, 
                                  FLOAT32 fFrequency, UINT32 nNumOctaves,
                                  FLOAT32 fAmplitude=1.0f, FLOAT32 fPersistence=0.5f );

private:
    INT32      m_p[ B + B + 2 ];
    FLOAT32    m_g1[ B + B + 2 ];
    FRMVECTOR2 m_g2[ B + B + 2 ];
    FRMVECTOR3 m_g3[ B + B + 2 ];
};


#endif // _FRM_NOISE_H_
