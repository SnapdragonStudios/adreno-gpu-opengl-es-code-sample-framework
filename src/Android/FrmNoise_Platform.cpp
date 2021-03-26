// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmNoise.h"


#define RAND FLOAT32( ( rand() % ( B + B ) ) - B )

#define setup(i, b0, b1, r0, r1)\
    INT32 b0 = INT32(i + N) & BM;\
    INT32 b1 = (b0 + 1) & BM;\
    FLOAT32 r0 = (i + N) - INT32(i + N);\
    FLOAT32 r1 = r0 - 1;


//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
CFrmPerlin::CFrmPerlin()
{
    for( INT32 i = 0; i < B; i++ )
    {
        m_p[i] = i;

        m_g1[i] = RAND / B;
		FRMVECTOR2 v2Rand = (FRMVECTOR2(RAND, RAND) / B);
		m_g2[i] = FrmVector2Normalize(v2Rand);
		FRMVECTOR3 v3Rand = (FRMVECTOR3(RAND, RAND, RAND) / B);
		m_g3[i] = FrmVector3Normalize(v3Rand);
    }

    for( INT32 i = 0; i < B; i++ )
    {
        INT32 r = rand() % B;
        INT32 tmp = m_p[i];
        m_p[i] = m_p[r];
        m_p[r] = tmp;
    }

    for( INT32 i = 0; i < B + 2; i++ )
    {
        m_p[B + i]  = m_p[i];
        m_g1[B + i] = m_g1[i];
        m_g2[B + i] = m_g2[i];
        m_g3[B + i] = m_g3[i];
    }
}


//--------------------------------------------------------------------------------------
// Name: Noise1D() / Noise2D() / Noise3D()
// Desc: Interpolated noise functions
//--------------------------------------------------------------------------------------
FLOAT32 CFrmPerlin::Noise1D( const FLOAT32 x )
{
    setup(x, bx0, bx1, rx0, rx1);
    FLOAT32 sx = FrmSCurve(rx0);

    return FrmLerp(sx, rx0 * m_g1[m_p[bx0]], rx1 * m_g1[m_p[bx1]]);
}


FLOAT32 CFrmPerlin::Noise2D( const FLOAT32 x, const FLOAT32 y )
{
    setup(x, bx0, bx1, rx0, rx1);
    setup(y, by0, by1, ry0, ry1);

    INT32 b00 = m_p[m_p[bx0] + by0];
    INT32 b10 = m_p[m_p[bx1] + by0];
    INT32 b01 = m_p[m_p[bx0] + by1];
    INT32 b11 = m_p[m_p[bx1] + by1];

    FLOAT32 sx = FrmSCurve(rx0);
    FLOAT32 sy = FrmSCurve(ry0);

	FRMVECTOR2 v2A = FRMVECTOR2(rx0, ry0);
	FRMVECTOR2 v2B = FRMVECTOR2(rx1, ry0);
    FLOAT32 a = FrmLerp(sx, FrmVector2Dot(m_g2[b00], v2A), FrmVector2Dot(m_g2[b10], v2B));
	v2A = FRMVECTOR2(rx0, ry1);
	v2B = FRMVECTOR2(rx1, ry1);
    FLOAT32 b = FrmLerp(sx, FrmVector2Dot(m_g2[b01], v2A), FrmVector2Dot(m_g2[b11], v2B));

    return FrmLerp(sy, a, b);
}


FLOAT32 CFrmPerlin::Noise3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z )
{
    setup(x, bx0, bx1, rx0, rx1);
    setup(y, by0, by1, ry0, ry1);
    setup(z, bz0, bz1, rz0, rz1);

    INT32 b00 = m_p[m_p[bx0] + by0];
    INT32 b10 = m_p[m_p[bx1] + by0];
    INT32 b01 = m_p[m_p[bx0] + by1];
    INT32 b11 = m_p[m_p[bx1] + by1];

    FLOAT32 sx = FrmSCurve(rx0);
    FLOAT32 sy = FrmSCurve(ry0);
    FLOAT32 sz = FrmSCurve(rz0);

	FRMVECTOR3 v3A = FRMVECTOR3(rx0, ry0, rz0);
	FRMVECTOR3 v3B = FRMVECTOR3(rx1, ry0, rz0);
    FLOAT32 a0 = FrmLerp(sx, FrmVector3Dot(m_g3[b00 + bz0], v3A), FrmVector3Dot(m_g3[b10 + bz0], v3B));
	v3A = FRMVECTOR3(rx0, ry1, rz0);
	v3B = FRMVECTOR3(rx1, ry1, rz0);
    FLOAT32 b0 = FrmLerp(sx, FrmVector3Dot(m_g3[b01 + bz0], v3A), FrmVector3Dot(m_g3[b11 + bz0], v3B));
    FLOAT32 c0 = FrmLerp(sy, a0, b0);

	v3A = FRMVECTOR3(rx0, ry0, rz1);
	v3B = FRMVECTOR3(rx1, ry0, rz1);
    FLOAT32 a1 = FrmLerp(sx, FrmVector3Dot(m_g3[b00 + bz1], v3A), FrmVector3Dot(m_g3[b10 + bz1], v3B));
	v3A = FRMVECTOR3(rx0, ry1, rz1);
	v3B = FRMVECTOR3(rx1, ry1, rz1);
    FLOAT32 b1 = FrmLerp(sx, FrmVector3Dot(m_g3[b01 + bz1], v3A), FrmVector3Dot(m_g3[b11 + bz1], v3B));
    FLOAT32 c1 = FrmLerp(sy, a1, b1);

    return FrmLerp(sz, c0, c1);
}


//--------------------------------------------------------------------------------------
// Name: Turbulence1D() / Turbulence2D() / Turbulence3D()
// Desc: Perlin noise functions
//--------------------------------------------------------------------------------------
FLOAT32 CFrmPerlin::Turbulence1D( const FLOAT32 x,
                                  FLOAT32 fFrequency, UINT32 nNumOctaves,
                                  FLOAT32 fAmplitude, FLOAT32 fPersistence )
{
    FLOAT32 fTotal = 0.0f;

    for( UINT32 i=0; i<nNumOctaves; i++ )
    {
        fTotal += Noise1D( fFrequency * x ) * fAmplitude;
        fFrequency *= 2.0f;
        fAmplitude *= fPersistence;
    }
    
    return fTotal;
}


FLOAT32 CFrmPerlin::Turbulence2D( const FLOAT32 x, const FLOAT32 y,
                                  FLOAT32 fFrequency, UINT32 nNumOctaves,
                                  FLOAT32 fAmplitude, FLOAT32 fPersistence )
{
    FLOAT32 fTotal = 0.0f;

    for( UINT32 i=0; i<nNumOctaves; i++ )
    {
        fTotal += Noise2D( fFrequency * x, fFrequency * y ) * fAmplitude;
        fFrequency *= 2.0f;
        fAmplitude *= fPersistence;
    }
    
    return fTotal;
}


FLOAT32 CFrmPerlin::Turbulence3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z,
                                  FLOAT32 fFrequency, UINT32 nNumOctaves,
                                  FLOAT32 fAmplitude, FLOAT32 fPersistence )
{
    FLOAT32 fTotal = 0.0f;

    for( UINT32 i=0; i<nNumOctaves; i++ )
    {
        fTotal += Noise3D( fFrequency * x, fFrequency * y, fFrequency * z ) * fAmplitude;
        fFrequency *= 2.0f;
        fAmplitude *= fPersistence;
    }
    
    return fTotal;
}


//--------------------------------------------------------------------------------------
// Name: TileableNoise1D() / TileableNoise2D() / TileableNoise3D()
// Desc: Tileable, interpolated noise functions
//--------------------------------------------------------------------------------------
FLOAT32 CFrmPerlin::TileableNoise1D(const FLOAT32 x, const FLOAT32 w)
{
    return( Noise1D( x     ) * ( w - x ) +
            Noise1D( x - w ) *       x   ) / w;
}

FLOAT32 CFrmPerlin::TileableNoise2D(const FLOAT32 x, const FLOAT32 y, const FLOAT32 w, const FLOAT32 h)
{
    return ( Noise2D( x,     y     ) * ( w - x )* ( h - y ) +
             Noise2D( x - w, y     ) *       x  * ( h - y ) +
             Noise2D( x,     y - h ) * ( w - x )*       y   +
             Noise2D( x - w, y - h ) *       x  *       y   ) / ( w * h );
}

FLOAT32 CFrmPerlin::TileableNoise3D(const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, const FLOAT32 w, const FLOAT32 h, const FLOAT32 d)
{
    return ( Noise3D( x,     y,     z     ) * ( w - x ) * ( h - y ) * ( d - z ) +
             Noise3D( x - w, y,     z     ) *       x   * ( h - y ) * ( d - z ) +
             Noise3D( x,     y - h, z     ) * ( w - x ) *       y   * ( d - z ) +
             Noise3D( x - w, y - h, z     ) *       x   *       y   * ( d - z ) + 
             Noise3D( x,     y,     z - d ) * ( w - x ) * ( h - y ) *       z   +
             Noise3D( x - w, y,     z - d ) *       x   * ( h - y ) *       z   +
             Noise3D( x,     y - h, z - d ) * ( w - x ) *       y   *       z   +
             Noise3D( x - w, y - h, z - d ) *       x   *       y   *       z   ) / ( w * h * d );
}


//--------------------------------------------------------------------------------------
// Name: TileableTurbulence1D() / TileableTurbulence2D() / TileableTurbulence3D()
// Desc: Tileable Perlin noise functions
//--------------------------------------------------------------------------------------
FLOAT32 CFrmPerlin::TileableTurbulence1D( const FLOAT32 x, 
                                          const FLOAT32 w, 
                                          FLOAT32 fFrequency, UINT32 nNumOctaves,
                                          FLOAT32 fAmplitude, FLOAT32 fPersistence )
{
    FLOAT32 fTotal = 0.0f;

    for( UINT32 i=0; i<nNumOctaves; i++ )
    {
        fTotal += TileableNoise1D( fFrequency * x, 
                                   fFrequency * w ) * fAmplitude;
        fFrequency *= 2.0f;
        fAmplitude *= fPersistence;
    }
    
    return fTotal;
}


FLOAT32 CFrmPerlin::TileableTurbulence2D( const FLOAT32 x, const FLOAT32 y, 
                                          const FLOAT32 w, const FLOAT32 h, 
                                          FLOAT32 fFrequency, UINT32 nNumOctaves,
                                          FLOAT32 fAmplitude, FLOAT32 fPersistence )
{
    FLOAT32 fTotal = 0.0f;

    for( UINT32 i=0; i<nNumOctaves; i++ )
    {
        fTotal += TileableNoise2D( fFrequency * x, fFrequency * y,
                                   fFrequency * w, fFrequency * h ) * fAmplitude;
        fFrequency *= 2.0f;
        fAmplitude *= fPersistence;
    }
    
    return fTotal;
}


FLOAT32 CFrmPerlin::TileableTurbulence3D( const FLOAT32 x, const FLOAT32 y, const FLOAT32 z, 
                                          const FLOAT32 w, const FLOAT32 h, const FLOAT32 d, 
                                          FLOAT32 fFrequency, UINT32 nNumOctaves,
                                          FLOAT32 fAmplitude, FLOAT32 fPersistence )
{
    FLOAT32 fTotal = 0.0f;

    for( UINT32 i=0; i<nNumOctaves; i++ )
    {
        fTotal += TileableNoise3D( fFrequency * x, fFrequency * y, fFrequency * z, 
                                   fFrequency * w, fFrequency * h, fFrequency * d ) * fAmplitude;
        fFrequency *= 2.0f;
        fAmplitude *= fPersistence;
    }
    
    return fTotal;
}

