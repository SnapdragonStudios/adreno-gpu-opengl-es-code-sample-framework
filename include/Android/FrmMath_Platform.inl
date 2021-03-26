// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <math.h>


//--------------------------------------------------------------------------------------
// Scalar math functions
//--------------------------------------------------------------------------------------
FLOAT32 FrmFloor( FLOAT32 value )      { return floorf( value ); }
VOID    FrmSrand( UINT32 seed )        { srand( seed ); }
FLOAT32 FrmRand()                      { return rand() * ( 1.0f / RAND_MAX ); }
FLOAT32 FrmPow( FLOAT32 x, FLOAT32 y ) { return powf( x, y ); }
FLOAT32 FrmSin( FLOAT32 deg )          { return sinf( deg ); }
FLOAT32 FrmCos( FLOAT32 deg )          { return cosf( deg ); }
FLOAT32 FrmAcos( FLOAT32 deg )         { return acosf( deg ); }
FLOAT32 FrmTan( FLOAT32 deg )          { return tanf( deg ); }
FLOAT32 FrmSqrt( FLOAT32 value )       { return sqrtf( value ); }
FLOAT32 FrmAbs( FLOAT32 value )        { return fabs( value ); }

    
//--------------------------------------------------------------------------------------
// Name: FrmLerp()
// Desc: 
//--------------------------------------------------------------------------------------
FLOAT32 FrmLerp( FLOAT32 t, FLOAT32 a, FLOAT32 b )
{
    return a + t * ( b - a );
}

FRMCOLOR FrmLerp( FLOAT32 t, const FRMCOLOR& vColor1, const FRMCOLOR& vColor2 )
{
    FRMCOLOR vResult;
    vResult.r = vColor1.r + (UINT8)( t * ( vColor2.r - vColor1.r ) );
    vResult.g = vColor1.g + (UINT8)( t * ( vColor2.g - vColor1.g ) );
    vResult.b = vColor1.b + (UINT8)( t * ( vColor2.b - vColor1.b ) );
    vResult.a = vColor1.a + (UINT8)( t * ( vColor2.a - vColor1.a ) );
    return vResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmSphrand()
// Desc: Returns a random point within a sphere centered at (0,0)
//--------------------------------------------------------------------------------------
FRMVECTOR3 FrmSphrand( FLOAT32 fRadius )
{
    // Calculate a random normalized vector
    FRMVECTOR3 vResult;
    vResult.x = 2.0f * FrmRand() - 1.0f;
    vResult.y = 2.0f * FrmRand() - 1.0f;
    vResult.z = 2.0f * FrmRand() - 1.0f;

    FLOAT32 fLength = FrmVector3Length( vResult );

    // Scale by a number between 0 and radius
    return fRadius * FrmRand() * vResult / fLength;
}


//--------------------------------------------------------------------------------------
// Name: ApplyVariance()
// Desc: Applies the specified amount of random variance to the specified value.
//--------------------------------------------------------------------------------------
FLOAT32 ApplyVariance( FLOAT32 fValue, FLOAT32 fVariance )
{
    return fValue - 0.5f * fVariance + FrmRand() * fVariance;
}

FLOAT32 FrmSCurve( const FLOAT32 t )
{
    return t * t * (3 - 2 * t);
}


//--------------------------------------------------------------------------------------
// Name: FrmVector2()
// Desc: Functions for FRMVECTOR2 vectors
//--------------------------------------------------------------------------------------
FLOAT32 FrmVector2Dot( FRMVECTOR2& vVector0, FRMVECTOR2& vVector1 )
{
    return ( vVector0.x * vVector1.x +
             vVector0.y * vVector1.y );
}


FLOAT32 FrmVector2Length( FRMVECTOR2& vVector )
{
    return FrmSqrt( vVector.x * vVector.x + vVector.y * vVector.y );
}


FRMVECTOR2 FrmVector2Normalize( FRMVECTOR2& vVector )
{
    FLOAT32 fLength = FrmVector2Length( vVector );
    if( fLength != 0.0f )
        fLength = 1.0f / fLength;
    
    return vVector * fLength;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector3()
// Desc: Functions for FRMVECTOR3 vectors
//--------------------------------------------------------------------------------------
FLOAT32 FrmVector3Dot( FRMVECTOR3& vVector0, FRMVECTOR3& vVector1 )
{
    return ( vVector0.x * vVector1.x +
             vVector0.y * vVector1.y +
             vVector0.z * vVector1.z );
}


FLOAT32 FrmVector3Length( FRMVECTOR3& vVector )
{
    return FrmSqrt( vVector.x * vVector.x +
                    vVector.y * vVector.y +
                    vVector.z * vVector.z );
}


FRMVECTOR3 FrmVector3Normalize( FRMVECTOR3& vVector )
{
    FLOAT32 fLength = FrmVector3Length( vVector );
    if( fLength != 0.0f )
        fLength = 1.0f / fLength;
    
    return vVector * fLength;
}


FRMVECTOR3 FrmVector3Mul( FRMVECTOR3& vVector, FLOAT32 fScalar )
{
    FRMVECTOR3 vResult;
    vResult.x = vVector.x * fScalar;
    vResult.y = vVector.y * fScalar;
    vResult.z = vVector.z * fScalar;
    return vResult;
}


FRMVECTOR3 FrmVector3Add( FRMVECTOR3& vVector0, FRMVECTOR3& vVector1 )
{
    FRMVECTOR3 vResult;
    vResult.x = vVector0.x + vVector1.x;
    vResult.y = vVector0.y + vVector1.y;
    vResult.z = vVector0.z + vVector1.z;
    return vResult;
}


FRMVECTOR3 FrmVector3Cross( FRMVECTOR3& vVector0, FRMVECTOR3& vVector1 )
{
    FRMVECTOR3 vResult;
    vResult.x = vVector0.y * vVector1.z - vVector0.z * vVector1.y;
    vResult.y = vVector0.z * vVector1.x - vVector0.x * vVector1.z;
    vResult.z = vVector0.x * vVector1.y - vVector0.y * vVector1.x;
    return vResult;
}


FRMVECTOR3 FrmVector3Lerp( FRMVECTOR3& v0, FRMVECTOR3& v1, FLOAT32 s )
{
    FRMVECTOR3 vLerp;
    vLerp.x = FrmLerp( s, v0.x, v1.x );
    vLerp.y = FrmLerp( s, v0.y, v1.y );
    vLerp.z = FrmLerp( s, v0.z, v1.z );
    return vLerp;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector3TransformNormal()
// Desc: Transforms a normal through a matrix
//--------------------------------------------------------------------------------------
FRMVECTOR3 FrmVector3TransformNormal( FRMVECTOR3& vSrcVector, FRMMATRIX4X4& matSrcMatrix )
{
    FRMVECTOR3 vDstVector;
    vDstVector.x = matSrcMatrix.M(0,0) * vSrcVector.x +
                   matSrcMatrix.M(1,0) * vSrcVector.y +
                   matSrcMatrix.M(2,0) * vSrcVector.z;
    vDstVector.y = matSrcMatrix.M(0,1) * vSrcVector.x +
                   matSrcMatrix.M(1,1) * vSrcVector.y +
                   matSrcMatrix.M(2,1) * vSrcVector.z;
    vDstVector.z = matSrcMatrix.M(0,2) * vSrcVector.x +
                   matSrcMatrix.M(1,2) * vSrcVector.y +
                   matSrcMatrix.M(2,2) * vSrcVector.z;
    return FrmVector3Normalize( vDstVector );
}


//--------------------------------------------------------------------------------------
// Name: FrmVector3TransformCoord()
// Desc: Transforms a 3-dimensional coordinate through a matrix
//--------------------------------------------------------------------------------------
FRMVECTOR3 FrmVector3TransformCoord( FRMVECTOR3& vSrcVector, FRMMATRIX4X4& matSrcMatrix )
{
    FRMVECTOR3 vDstVector;
    vDstVector.x = matSrcMatrix.M(0,0) * vSrcVector.x +
                   matSrcMatrix.M(1,0) * vSrcVector.y +
                   matSrcMatrix.M(2,0) * vSrcVector.z +
                   matSrcMatrix.M(3,0);
    vDstVector.y = matSrcMatrix.M(0,1) * vSrcVector.x +
                   matSrcMatrix.M(1,1) * vSrcVector.y +
                   matSrcMatrix.M(2,1) * vSrcVector.z +
                   matSrcMatrix.M(3,1);
    vDstVector.z = matSrcMatrix.M(0,2) * vSrcVector.x +
                   matSrcMatrix.M(1,2) * vSrcVector.y +
                   matSrcMatrix.M(2,2) * vSrcVector.z +
                   matSrcMatrix.M(3,2);
    return vDstVector;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector3Transform()
// Desc: Transforms a vector through a matrix
//--------------------------------------------------------------------------------------
FRMVECTOR4 FrmVector3Transform( FRMVECTOR3& vSrcVector, FRMMATRIX4X4& matSrcMatrix )
{
    FRMVECTOR4 vDstVector;
    vDstVector.x = matSrcMatrix.M(0,0) * vSrcVector.x +
                   matSrcMatrix.M(1,0) * vSrcVector.y +
                   matSrcMatrix.M(2,0) * vSrcVector.z +
                   matSrcMatrix.M(3,0);
    vDstVector.y = matSrcMatrix.M(0,1) * vSrcVector.x +
                   matSrcMatrix.M(1,1) * vSrcVector.y +
                   matSrcMatrix.M(2,1) * vSrcVector.z +
                   matSrcMatrix.M(3,1);
    vDstVector.z = matSrcMatrix.M(0,2) * vSrcVector.x +
                   matSrcMatrix.M(1,2) * vSrcVector.y +
                   matSrcMatrix.M(2,2) * vSrcVector.z +
                   matSrcMatrix.M(3,2);
    vDstVector.w = matSrcMatrix.M(0,3) * vSrcVector.x +
                   matSrcMatrix.M(1,3) * vSrcVector.y +
                   matSrcMatrix.M(2,3) * vSrcVector.z +
                   matSrcMatrix.M(3,3);
    return vDstVector;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector4()
// Desc: Functions for FRMVECTOR4 vectors
//--------------------------------------------------------------------------------------
FLOAT32 FrmVector4Dot( FRMVECTOR4& vVector0, FRMVECTOR4& vVector1 )
{
    return ( vVector0.x * vVector1.x +
             vVector0.y * vVector1.y +
             vVector0.z * vVector1.z +
             vVector0.w * vVector1.w );
}


FLOAT32 FrmVector4Length( FRMVECTOR4& vVector )
{
    return ( vVector.x * vVector.x +
             vVector.y * vVector.y +
             vVector.z * vVector.z +
             vVector.w * vVector.w );
}


FRMVECTOR4 FrmVector4Normalize( FRMVECTOR4& vVector )
{
    FLOAT32 fLength = FrmVector4Length( vVector );
    if( fLength != 0.0f )
        fLength = 1.0f / FrmSqrt( fLength );
    
    return vVector * fLength;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector4SLerp()
// Desc: Spherical Lerp for quaternion-based rotations
//--------------------------------------------------------------------------------------
FRMVECTOR4 FrmVector4SLerp( FRMVECTOR4& q0, FRMVECTOR4& q1, FLOAT32 s )
{
    FLOAT32 fCosTheta = FrmVector4Dot( q0, q1 );
    if( fCosTheta < 0.0f )
    {
        q0        *= -1.0f;
        fCosTheta *= -1.0f;
    }

    FLOAT32 s0 = 1.0f - s;
    FLOAT32 s1 = s;

    // Linear interploation
    if( fCosTheta > +0.95f )
        return s0 * q0 + s1 * q1;

    // Spherical interpolation
    FLOAT32 fTheta = FrmAcos( fCosTheta );
    s0 = FrmSin( fTheta * s0 );
    s1 = FrmSin( fTheta * s1 );
    return ( s0 * q0 + s1 * q1 ) / FrmSin( fTheta );
}


//--------------------------------------------------------------------------------------
// Name: FrmQuaternionMultiply()
// Desc: Multiple two quaternions and return the result
//--------------------------------------------------------------------------------------
FRMVECTOR4 FrmQuaternionMultiply( FRMVECTOR4& q0, FRMVECTOR4& q1 )
{
    FRMVECTOR4 qResult;
    qResult.x = q0.w*q1.x + q0.x*q1.w + q0.y*q1.z - q0.z*q1.y;
    qResult.y = q0.w*q1.y + q0.y*q1.w + q0.z*q1.x - q0.x*q1.z;
    qResult.z = q0.w*q1.z + q0.z*q1.w + q0.x*q1.y - q0.y*q1.x;
    qResult.w = q0.w*q1.w - q0.x*q1.x - q0.y*q1.y - q0.z*q1.z;
    return qResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmVector4Transform()
// Desc: Transforms a vector through a matrix
//--------------------------------------------------------------------------------------
FRMVECTOR4 FrmVector4Transform( FRMVECTOR4& vSrcVector, FRMMATRIX4X4& matSrcMatrix )
{
    FRMVECTOR4 vDstVector;
    vDstVector.x = matSrcMatrix.M(0,0) * vSrcVector.x +
                   matSrcMatrix.M(1,0) * vSrcVector.y +
                   matSrcMatrix.M(2,0) * vSrcVector.z +
                   matSrcMatrix.M(3,0) * vSrcVector.w;
    vDstVector.y = matSrcMatrix.M(0,1) * vSrcVector.x +
                   matSrcMatrix.M(1,1) * vSrcVector.y +
                   matSrcMatrix.M(2,1) * vSrcVector.z +
                   matSrcMatrix.M(3,1) * vSrcVector.w;
    vDstVector.z = matSrcMatrix.M(0,2) * vSrcVector.x +
                   matSrcMatrix.M(1,2) * vSrcVector.y +
                   matSrcMatrix.M(2,2) * vSrcVector.z +
                   matSrcMatrix.M(3,2) * vSrcVector.w;
    vDstVector.w = matSrcMatrix.M(0,3) * vSrcVector.x +
                   matSrcMatrix.M(1,3) * vSrcVector.y +
                   matSrcMatrix.M(2,3) * vSrcVector.z +
                   matSrcMatrix.M(3,3) * vSrcVector.w;
    return vDstVector;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixIdentity()
// Desc: Returns the 4x4 identity matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixIdentity()
{
    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = 1.0f;
    matResult.M(0,1) = 0.0f;
    matResult.M(0,2) = 0.0f;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = 0.0f;
    matResult.M(1,1) = 1.0f;
    matResult.M(1,2) = 0.0f;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = 0.0f;
    matResult.M(2,1) = 0.0f;
    matResult.M(2,2) = 1.0f;
    matResult.M(2,3) = 0.0f;

    matResult.M(3,0) = 0.0f;
    matResult.M(3,1) = 0.0f;
    matResult.M(3,2) = 0.0f;
    matResult.M(3,3) = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixLookAtLH()
// Desc: Returns a left-handed lookat matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixLookAtLH( FRMVECTOR3& vPosition, FRMVECTOR3& vLookAt, FRMVECTOR3& vUp )
{
    FRMVECTOR3 ZAxis = vLookAt - vPosition;
    FRMVECTOR3 XAxis = FrmVector3Cross( vUp, ZAxis );
    FRMVECTOR3 YAxis = FrmVector3Cross( ZAxis, XAxis );

    XAxis = FrmVector3Normalize( XAxis );
    YAxis = FrmVector3Normalize( YAxis );
    ZAxis = FrmVector3Normalize( ZAxis );

    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = XAxis.x;
    matResult.M(0,1) = YAxis.x;
    matResult.M(0,2) = ZAxis.x;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = XAxis.y;
    matResult.M(1,1) = YAxis.y;
    matResult.M(1,2) = ZAxis.y;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = XAxis.z;
    matResult.M(2,1) = YAxis.z;
    matResult.M(2,2) = ZAxis.z;
    matResult.M(2,3) = 0.0f;

    matResult.M(3,0) = -FrmVector3Dot( XAxis, vPosition );
    matResult.M(3,1) = -FrmVector3Dot( YAxis, vPosition );
    matResult.M(3,2) = -FrmVector3Dot( ZAxis, vPosition );
    matResult.M(3,3) = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixLookAtRH()
// Desc: Returns a right-handed lookat matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixLookAtRH( FRMVECTOR3& vPosition, FRMVECTOR3& vLookAt, FRMVECTOR3& vUp )
{
    FRMVECTOR3 ZAxis = vLookAt - vPosition;
    FRMVECTOR3 XAxis = FrmVector3Cross( vUp, ZAxis );
    FRMVECTOR3 YAxis = FrmVector3Cross( ZAxis, XAxis );
    XAxis = FrmVector3Normalize( XAxis );
    YAxis = FrmVector3Normalize( YAxis );
    ZAxis = FrmVector3Normalize( ZAxis );

    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = -XAxis.x;
    matResult.M(0,1) = +YAxis.x;
    matResult.M(0,2) = -ZAxis.x;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = -XAxis.y;
    matResult.M(1,1) = +YAxis.y;
    matResult.M(1,2) = -ZAxis.y;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = -XAxis.z;
    matResult.M(2,1) = +YAxis.z;
    matResult.M(2,2) = -ZAxis.z;
    matResult.M(2,3) = 0.0f;

    matResult.M(3,0) = +FrmVector3Dot( XAxis, vPosition );
    matResult.M(3,1) = -FrmVector3Dot( YAxis, vPosition );
    matResult.M(3,2) = +FrmVector3Dot( ZAxis, vPosition );
    matResult.M(3,3) = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixPerspectiveFovLH()
// Desc: Sets the matrix to a perspective projection
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixPerspectiveFovLH( FLOAT32 fFOVy, FLOAT32 fAspect, 
                                        FLOAT32 zNear, FLOAT32 zFar )
{
    FLOAT32 SinFov = FrmSin( 0.5f * fFOVy );
    FLOAT32 CosFov = FrmCos( 0.5f * fFOVy );
    FLOAT32 h = CosFov / SinFov;
    FLOAT32 w  = h / fAspect;

    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = w;
    matResult.M(0,1) = 0.0f;
    matResult.M(0,2) = 0.0f;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = 0.0f;
    matResult.M(1,1) = h;
    matResult.M(1,2) = 0.0f;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = 0.0f;
    matResult.M(2,1) = 0.0f;
    matResult.M(2,2) = -( zFar+zNear ) / ( zFar-zNear );
    matResult.M(2,3) = -( 2.0f*zFar*zNear ) / ( zFar-zNear );

    matResult.M(3,0) = 0.0f;
    matResult.M(3,1) = 0.0f;
    matResult.M(3,2) = -1.0f;
    matResult.M(3,3) = 0.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixPerspectiveFovRH()
// Desc: Sets the matrix to a perspective projection
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixPerspectiveFovRH( FLOAT32 fFOVy, FLOAT32 fAspect,
                                        FLOAT32 zNear, FLOAT32 zFar )
{
    FLOAT32 SinFov = FrmSin( 0.5f * fFOVy );
    FLOAT32 CosFov = FrmCos( 0.5f * fFOVy );
    FLOAT32 h = CosFov / SinFov;
    FLOAT32 w  = h / fAspect;

    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = w;
    matResult.M(0,1) = 0.0f;
    matResult.M(0,2) = 0.0f;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = 0.0f;
    matResult.M(1,1) = h;
    matResult.M(1,2) = 0.0f;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = 0.0f;
    matResult.M(2,1) = 0.0f;
    matResult.M(2,2) = -( zFar ) / ( zFar-zNear );
    matResult.M(2,3) = -1.0f;

    matResult.M(3,0) = 0.0f;
    matResult.M(3,1) = 0.0f;
    matResult.M(3,2) = -( zFar * zNear ) / ( zFar-zNear );
    matResult.M(3,3) = 0.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixMultiply()
// Desc: Multiplies two matrices.
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixMultiply( FRMMATRIX4X4& matSrcMatrixA, FRMMATRIX4X4& matSrcMatrixB )
{
    FRMMATRIX4X4  DstMatrix;
    FRMMATRIX4X4* pSrc1 = &matSrcMatrixA;
    FRMMATRIX4X4* pSrc2 = &matSrcMatrixB;

    DstMatrix.M(0,0) = pSrc1->M(0,0)*pSrc2->M(0,0) + pSrc1->M(0,1)*pSrc2->M(1,0) + pSrc1->M(0,2)*pSrc2->M(2,0) + pSrc1->M(0,3)*pSrc2->M(3,0);
    DstMatrix.M(0,1) = pSrc1->M(0,0)*pSrc2->M(0,1) + pSrc1->M(0,1)*pSrc2->M(1,1) + pSrc1->M(0,2)*pSrc2->M(2,1) + pSrc1->M(0,3)*pSrc2->M(3,1);
    DstMatrix.M(0,2) = pSrc1->M(0,0)*pSrc2->M(0,2) + pSrc1->M(0,1)*pSrc2->M(1,2) + pSrc1->M(0,2)*pSrc2->M(2,2) + pSrc1->M(0,3)*pSrc2->M(3,2);
    DstMatrix.M(0,3) = pSrc1->M(0,0)*pSrc2->M(0,3) + pSrc1->M(0,1)*pSrc2->M(1,3) + pSrc1->M(0,2)*pSrc2->M(2,3) + pSrc1->M(0,3)*pSrc2->M(3,3);
    DstMatrix.M(1,0) = pSrc1->M(1,0)*pSrc2->M(0,0) + pSrc1->M(1,1)*pSrc2->M(1,0) + pSrc1->M(1,2)*pSrc2->M(2,0) + pSrc1->M(1,3)*pSrc2->M(3,0);
    DstMatrix.M(1,1) = pSrc1->M(1,0)*pSrc2->M(0,1) + pSrc1->M(1,1)*pSrc2->M(1,1) + pSrc1->M(1,2)*pSrc2->M(2,1) + pSrc1->M(1,3)*pSrc2->M(3,1);
    DstMatrix.M(1,2) = pSrc1->M(1,0)*pSrc2->M(0,2) + pSrc1->M(1,1)*pSrc2->M(1,2) + pSrc1->M(1,2)*pSrc2->M(2,2) + pSrc1->M(1,3)*pSrc2->M(3,2);
    DstMatrix.M(1,3) = pSrc1->M(1,0)*pSrc2->M(0,3) + pSrc1->M(1,1)*pSrc2->M(1,3) + pSrc1->M(1,2)*pSrc2->M(2,3) + pSrc1->M(1,3)*pSrc2->M(3,3);
    DstMatrix.M(2,0) = pSrc1->M(2,0)*pSrc2->M(0,0) + pSrc1->M(2,1)*pSrc2->M(1,0) + pSrc1->M(2,2)*pSrc2->M(2,0) + pSrc1->M(2,3)*pSrc2->M(3,0);
    DstMatrix.M(2,1) = pSrc1->M(2,0)*pSrc2->M(0,1) + pSrc1->M(2,1)*pSrc2->M(1,1) + pSrc1->M(2,2)*pSrc2->M(2,1) + pSrc1->M(2,3)*pSrc2->M(3,1);
    DstMatrix.M(2,2) = pSrc1->M(2,0)*pSrc2->M(0,2) + pSrc1->M(2,1)*pSrc2->M(1,2) + pSrc1->M(2,2)*pSrc2->M(2,2) + pSrc1->M(2,3)*pSrc2->M(3,2);
    DstMatrix.M(2,3) = pSrc1->M(2,0)*pSrc2->M(0,3) + pSrc1->M(2,1)*pSrc2->M(1,3) + pSrc1->M(2,2)*pSrc2->M(2,3) + pSrc1->M(2,3)*pSrc2->M(3,3);
    DstMatrix.M(3,0) = pSrc1->M(3,0)*pSrc2->M(0,0) + pSrc1->M(3,1)*pSrc2->M(1,0) + pSrc1->M(3,2)*pSrc2->M(2,0) + pSrc1->M(3,3)*pSrc2->M(3,0);
    DstMatrix.M(3,1) = pSrc1->M(3,0)*pSrc2->M(0,1) + pSrc1->M(3,1)*pSrc2->M(1,1) + pSrc1->M(3,2)*pSrc2->M(2,1) + pSrc1->M(3,3)*pSrc2->M(3,1);
    DstMatrix.M(3,2) = pSrc1->M(3,0)*pSrc2->M(0,2) + pSrc1->M(3,1)*pSrc2->M(1,2) + pSrc1->M(3,2)*pSrc2->M(2,2) + pSrc1->M(3,3)*pSrc2->M(3,2);
    DstMatrix.M(3,3) = pSrc1->M(3,0)*pSrc2->M(0,3) + pSrc1->M(3,1)*pSrc2->M(1,3) + pSrc1->M(3,2)*pSrc2->M(2,3) + pSrc1->M(3,3)*pSrc2->M(3,3);

    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: DET2X2()
// Desc: Helper function to compute the determinant of a 2x2 matrix
//--------------------------------------------------------------------------------------
__inline FLOAT32 DET2X2( FLOAT32 m00, FLOAT32 m01, 
                         FLOAT32 m10, FLOAT32 m11 )
{
    return m00 * m11 - m01 * m10;
}


//--------------------------------------------------------------------------------------
// Name: DET3X3()
// Desc: Helper function to compute the determinant of a 3x3 matrix
//--------------------------------------------------------------------------------------
__inline FLOAT32 DET3X3( FLOAT32 m00, FLOAT32 m01, FLOAT32 m02,
                         FLOAT32 m10, FLOAT32 m11, FLOAT32 m12,
                         FLOAT32 m20, FLOAT32 m21, FLOAT32 m22 )
{
    return m00 * DET2X2( m11, m12, m21, m22 ) -
           m10 * DET2X2( m01, m02, m21, m22 ) +
           m20 * DET2X2( m01, m02, m11, m12 );
}


//--------------------------------------------------------------------------------------
// Name: DET4X4()
// Desc: Helper function to compute the determinant of a 4X4 matrix
//--------------------------------------------------------------------------------------
__inline FLOAT32 DET4X4( FLOAT32 m00, FLOAT32 m01, FLOAT32 m02, FLOAT32 m03,
                         FLOAT32 m10, FLOAT32 m11, FLOAT32 m12, FLOAT32 m13,
                         FLOAT32 m20, FLOAT32 m21, FLOAT32 m22, FLOAT32 m23,
                         FLOAT32 m30, FLOAT32 m31, FLOAT32 m32, FLOAT32 m33 )
{
    return m00 * DET3X3( m11, m12, m13, m21, m22, m23, m31, m32, m33 ) -
           m10 * DET3X3( m01, m02, m03, m21, m22, m23, m31, m32, m33 ) +
           m20 * DET3X3( m01, m02, m03, m11, m12, m13, m31, m32, m33 ) -
           m30 * DET3X3( m01, m02, m03, m11, m12, m13, m21, m22, m23 );
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixNormalOrthonormal()
// Desc: Generates a matrix for transforming normals.
//--------------------------------------------------------------------------------------
FRMMATRIX3X3 FrmMatrixNormalOrthonormal( FRMMATRIX4X4& matSrcMatrix )
{
    // Note that we assume that the input matrix is orthonormal, so that we just need
    // to extract the 3x3 rotation core of the 4x4 source matrix

    FRMMATRIX3X3 matResult;

    matResult.M(0,0) = matSrcMatrix.M(0,0);
    matResult.M(0,1) = matSrcMatrix.M(0,1);
    matResult.M(0,2) = matSrcMatrix.M(0,2);
    
    matResult.M(1,0) = matSrcMatrix.M(1,0);
    matResult.M(1,1) = matSrcMatrix.M(1,1);
    matResult.M(1,2) = matSrcMatrix.M(1,2);
    
    matResult.M(2,0) = matSrcMatrix.M(2,0);
    matResult.M(2,1) = matSrcMatrix.M(2,1);
    matResult.M(2,2) = matSrcMatrix.M(2,2);

    return matResult;
}

//--------------------------------------------------------------------------------------
// Name: FrmMatrixNormal()
// Desc: Generates a matrix for transforming normals.
//--------------------------------------------------------------------------------------
FRMMATRIX3X3 FrmMatrixNormal( FRMMATRIX4X4& matSrcMatrix )
{
    // The generic case for the normal matrix is the transpose of the matrix inverse.
    // The matrix inverse, meanwhile, is the adjoint of the matrix scale by its
    // determinant

    FRMMATRIX3X3 DstMatrix;
    FRMMATRIX4X4* s = &matSrcMatrix;

    // Compute the matrix' determinant
    FLOAT32 fDeterminant = DET3X3( s->M(0,0), s->M(0,1), s->M(0,2), 
                                   s->M(1,0), s->M(1,1), s->M(1,2), 
                                   s->M(2,0), s->M(2,1), s->M(2,2) );
    FLOAT32 fScale = 1.0f / fDeterminant;

    // Divide the adjoint of the matrix by its determinant and transpose the result
    DstMatrix.M(0,0) = +DET2X2( s->M(1,1), s->M(1,2),  s->M(2,1), s->M(2,2) ) * fScale;
    DstMatrix.M(0,1) = -DET2X2( s->M(1,0), s->M(1,2),  s->M(2,0), s->M(2,2) ) * fScale;
    DstMatrix.M(0,2) = +DET2X2( s->M(1,0), s->M(1,1),  s->M(2,0), s->M(2,1) ) * fScale;
    
    DstMatrix.M(1,0) = -DET2X2( s->M(0,1), s->M(0,2),  s->M(2,1), s->M(2,2) ) * fScale;
    DstMatrix.M(1,1) = +DET2X2( s->M(0,0), s->M(0,2),  s->M(2,0), s->M(2,2) ) * fScale;
    DstMatrix.M(1,2) = -DET2X2( s->M(0,0), s->M(0,1),  s->M(2,0), s->M(2,1) ) * fScale;
    
    DstMatrix.M(2,0) = +DET2X2( s->M(0,1), s->M(0,2),  s->M(1,1), s->M(1,2) ) * fScale;
    DstMatrix.M(2,1) = -DET2X2( s->M(0,0), s->M(0,2),  s->M(1,0), s->M(1,2) ) * fScale;
    DstMatrix.M(2,2) = +DET2X2( s->M(0,0), s->M(0,1),  s->M(1,0), s->M(1,1) ) * fScale;
    
    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixInverse()
// Desc: Computes the inverse of an orthonormal matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixInverse( FRMMATRIX4X4& matSrcMatrix )
{
    FRMMATRIX4X4 DstMatrix;
    FRMMATRIX4X4* s = &matSrcMatrix;

    // Compute the matrix' determinant
    FLOAT32 fDeterminant = DET4X4( s->M(0,0), s->M(0,1), s->M(0,2), s->M(0,3),
                                   s->M(1,0), s->M(1,1), s->M(1,2), s->M(1,3),
                                   s->M(2,0), s->M(2,1), s->M(2,2), s->M(2,3),
                                   s->M(3,0), s->M(3,1), s->M(3,2), s->M(3,3) );
    FLOAT32 fScale = 1.0f / fDeterminant;

    // Divide the adjoint of the matrix by its determinant
    DstMatrix.M(0,0) = +DET3X3( s->M(1,1), s->M(1,2), s->M(1,3),   s->M(2,1), s->M(2,2), s->M(2,3),   s->M(3,1), s->M(3,2), s->M(3,3) ) * fScale;
    DstMatrix.M(0,1) = -DET3X3( s->M(1,0), s->M(1,2), s->M(1,3),   s->M(2,0), s->M(2,2), s->M(2,3),   s->M(3,0), s->M(3,2), s->M(3,3) ) * fScale;
    DstMatrix.M(0,2) = +DET3X3( s->M(1,0), s->M(1,1), s->M(1,3),   s->M(2,0), s->M(2,1), s->M(2,3),   s->M(3,0), s->M(3,1), s->M(3,3) ) * fScale;
    DstMatrix.M(0,3) = -DET3X3( s->M(1,0), s->M(1,1), s->M(1,2),   s->M(2,0), s->M(2,1), s->M(2,2),   s->M(3,0), s->M(3,1), s->M(3,2) ) * fScale;
    
    DstMatrix.M(1,0) = -DET3X3( s->M(0,1), s->M(0,2), s->M(0,3),   s->M(2,1), s->M(2,2), s->M(2,3),   s->M(3,1), s->M(3,2), s->M(3,3) ) * fScale;
    DstMatrix.M(1,1) = +DET3X3( s->M(0,0), s->M(0,2), s->M(0,3),   s->M(2,0), s->M(2,2), s->M(2,3),   s->M(3,0), s->M(3,2), s->M(3,3) ) * fScale;
    DstMatrix.M(1,2) = -DET3X3( s->M(0,0), s->M(0,1), s->M(0,3),   s->M(2,0), s->M(2,1), s->M(2,3),   s->M(3,0), s->M(3,1), s->M(3,3) ) * fScale;
    DstMatrix.M(1,3) = +DET3X3( s->M(0,0), s->M(0,1), s->M(0,2),   s->M(2,0), s->M(2,1), s->M(2,2),   s->M(3,0), s->M(3,1), s->M(3,2) ) * fScale;
    
    DstMatrix.M(2,0) = +DET3X3( s->M(0,1), s->M(0,2), s->M(0,3),   s->M(1,1), s->M(1,2), s->M(1,3),   s->M(3,1), s->M(3,2), s->M(3,3) ) * fScale;
    DstMatrix.M(2,1) = -DET3X3( s->M(0,0), s->M(0,2), s->M(0,3),   s->M(1,0), s->M(1,2), s->M(1,3),   s->M(3,0), s->M(3,2), s->M(3,3) ) * fScale;
    DstMatrix.M(2,2) = +DET3X3( s->M(0,0), s->M(0,1), s->M(0,3),   s->M(1,0), s->M(1,1), s->M(1,3),   s->M(3,0), s->M(3,1), s->M(3,3) ) * fScale;
    DstMatrix.M(2,3) = -DET3X3( s->M(0,0), s->M(0,1), s->M(0,2),   s->M(1,0), s->M(1,1), s->M(1,2),   s->M(3,0), s->M(3,1), s->M(3,2) ) * fScale;
    
    DstMatrix.M(3,0) = -DET3X3( s->M(0,1), s->M(0,2), s->M(0,3),   s->M(1,1), s->M(1,2), s->M(1,3),   s->M(2,1), s->M(2,2), s->M(2,3) ) * fScale;
    DstMatrix.M(3,1) = +DET3X3( s->M(0,0), s->M(0,2), s->M(0,3),   s->M(1,0), s->M(1,2), s->M(1,3),   s->M(2,0), s->M(2,2), s->M(2,3) ) * fScale;
    DstMatrix.M(3,2) = -DET3X3( s->M(0,0), s->M(0,1), s->M(0,3),   s->M(1,0), s->M(1,1), s->M(1,3),   s->M(2,0), s->M(2,1), s->M(2,3) ) * fScale;
    DstMatrix.M(3,3) = +DET3X3( s->M(0,0), s->M(0,1), s->M(0,2),   s->M(1,0), s->M(1,1), s->M(1,2),   s->M(2,0), s->M(2,1), s->M(2,2) ) * fScale;

    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixInverseOrthonormal()
// Desc: Computes the inverse of an orthonormal matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixInverseOrthonormal( FRMMATRIX4X4& matSrcMatrix )
{
    FRMMATRIX4X4 DstMatrix;
    FRMMATRIX4X4* s = &matSrcMatrix;

    // Transpose the 3x3 rotation submatrix
    DstMatrix.M(0,0) = s->M(0,0);  
    DstMatrix.M(0,1) = s->M(1,0);  
    DstMatrix.M(0,2) = s->M(2,0);
    DstMatrix.M(0,3) = 0.0f; 
    
    DstMatrix.M(1,0) = s->M(0,1);
    DstMatrix.M(1,1) = s->M(1,1);
    DstMatrix.M(1,2) = s->M(2,1);
    DstMatrix.M(1,3) = 0.0f; 
    
    DstMatrix.M(2,0) = s->M(0,2);
    DstMatrix.M(2,1) = s->M(1,2);
    DstMatrix.M(2,2) = s->M(2,2);
    DstMatrix.M(2,3) = 0.0f; 

    // Invert the matrix translation
    DstMatrix.M(0,0) = -s->M(3,0)*s->M(0,0) - s->M(3,1)*s->M(0,1) - s->M(3,2)*s->M(0,2);
    DstMatrix.M(1,1) = -s->M(3,0)*s->M(1,0) - s->M(3,1)*s->M(1,1) - s->M(3,2)*s->M(1,2);
    DstMatrix.M(2,2) = -s->M(3,0)*s->M(2,0) - s->M(3,1)*s->M(2,1) - s->M(3,2)*s->M(2,2);
    DstMatrix.M(3,3) = 1.0f;

    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixTranspose()
// Desc: Return the transpose of a matrix
//--------------------------------------------------------------------------------------
FRMMATRIX3X3 FrmMatrixTranspose( FRMMATRIX3X3& matSrcMatrix )
{
    FRMMATRIX3X3 DstMatrix;

    DstMatrix.M(0,0) = matSrcMatrix.M(0,0);
    DstMatrix.M(0,1) = matSrcMatrix.M(1,0);
    DstMatrix.M(0,2) = matSrcMatrix.M(2,0);

    DstMatrix.M(1,0) = matSrcMatrix.M(0,1);
    DstMatrix.M(1,1) = matSrcMatrix.M(1,1);
    DstMatrix.M(1,2) = matSrcMatrix.M(2,1);

    DstMatrix.M(2,0) = matSrcMatrix.M(0,2);
    DstMatrix.M(2,1) = matSrcMatrix.M(1,2);
    DstMatrix.M(2,2) = matSrcMatrix.M(2,2);

    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixTranspose()
// Desc: Return the transpose of a matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixTranspose( FRMMATRIX4X4& matSrcMatrix )
{
    FRMMATRIX4X4 DstMatrix;

    DstMatrix.M(0,0) = matSrcMatrix.M(0,0);
    DstMatrix.M(0,1) = matSrcMatrix.M(1,0);
    DstMatrix.M(0,2) = matSrcMatrix.M(2,0);
    DstMatrix.M(0,3) = matSrcMatrix.M(3,0);

    DstMatrix.M(1,0) = matSrcMatrix.M(0,1);
    DstMatrix.M(1,1) = matSrcMatrix.M(1,1);
    DstMatrix.M(1,2) = matSrcMatrix.M(2,1);
    DstMatrix.M(1,3) = matSrcMatrix.M(3,1);

    DstMatrix.M(2,0) = matSrcMatrix.M(0,2);
    DstMatrix.M(2,1) = matSrcMatrix.M(1,2);
    DstMatrix.M(2,2) = matSrcMatrix.M(2,2);
    DstMatrix.M(2,3) = matSrcMatrix.M(3,2);

    DstMatrix.M(3,0) = matSrcMatrix.M(0,3);
    DstMatrix.M(3,1) = matSrcMatrix.M(1,3);
    DstMatrix.M(3,2) = matSrcMatrix.M(2,3);
    DstMatrix.M(3,3) = matSrcMatrix.M(3,3);

    return DstMatrix;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixTranslate()
// Desc: Builds a translation matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixTranslate( FRMVECTOR3& vVector )
{
    return FrmMatrixTranslate( vVector.x, vVector.y, vVector.z );
}

FRMMATRIX4X4 FrmMatrixTranslate( FLOAT32 tx, FLOAT32 ty, FLOAT32 tz )
{
    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = 1.0f;
    matResult.M(0,1) = 0.0f;
    matResult.M(0,2) = 0.0f;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = 0.0f;
    matResult.M(1,1) = 1.0f;
    matResult.M(1,2) = 0.0f;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = 0.0f;
    matResult.M(2,1) = 0.0f;
    matResult.M(2,2) = 1.0f;
    matResult.M(2,3) = 0.0f;

    matResult.M(3,0) = tx;
    matResult.M(3,1) = ty;
    matResult.M(3,2) = tz;
    matResult.M(3,3) = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixScale()
// Desc: Builds a scaling matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixScale( FRMVECTOR3& vVector )
{
    return FrmMatrixScale( vVector.x, vVector.y, vVector.z );
}

FRMMATRIX4X4 FrmMatrixScale( FLOAT32 sx, FLOAT32 sy, FLOAT32 sz )
{
    FRMMATRIX4X4 matResult;

    matResult.M(0,0) = sx;
    matResult.M(0,1) = 0.0f;
    matResult.M(0,2) = 0.0f;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = 0.0f;
    matResult.M(1,1) = sy;
    matResult.M(1,2) = 0.0f;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = 0.0f;
    matResult.M(2,1) = 0.0f;
    matResult.M(2,2) = sz;
    matResult.M(2,3) = 0.0f;

    matResult.M(3,0) = 0.0f;
    matResult.M(3,1) = 0.0f;
    matResult.M(3,2) = 0.0f;
    matResult.M(3,3) = 1.0f;

    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixRotate()
// Desc: Builds a rotation matrix
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixRotate( FLOAT32 fAngle, FRMVECTOR3& vAxis )
{
    return FrmMatrixRotate( fAngle, vAxis.x, vAxis.y, vAxis.z );
}

FRMMATRIX4X4 FrmMatrixRotate( FLOAT32 fAngle, FLOAT32 rx, FLOAT32 ry, FLOAT32 rz )
{
    FLOAT32 c = FrmCos( fAngle );
    FLOAT32 s = FrmSin( fAngle );
    FLOAT32 t = 1 - c;

    // Normalize the rotation axis
    FLOAT32 len = FrmSqrt( rx*rx + ry*ry + rz*rz );
    rx /= len;
    ry /= len;
    rz /= len;

    // Build the rotation matrix
    FRMMATRIX4X4 matResult;
    matResult.M(0,0) = t * rx * rx + c;
    matResult.M(0,1) = t * ry * rx + s * rz;
    matResult.M(0,2) = t * rz * rx - s * ry;
    matResult.M(0,3) = 0.0f;

    matResult.M(1,0) = t * rx * ry - s * rz;
    matResult.M(1,1) = t * ry * ry + c;
    matResult.M(1,2) = t * rz * ry + s * rx;
    matResult.M(1,3) = 0.0f;

    matResult.M(2,0) = t * rx * rz + s * ry;
    matResult.M(2,1) = t * ry * rz - s * rx;
    matResult.M(2,2) = t * rz * rz + c;
    matResult.M(2,3) = 0.0f;

    matResult.M(3,0) = 0.0f;
    matResult.M(3,1) = 0.0f;
    matResult.M(3,2) = 0.0f;
    matResult.M(3,3) = 1.0f;
    
    return matResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixRotate()
// Desc: Builds a rotation matrix from a quaternion
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixRotate( FRMVECTOR4& q )
{
    FRMMATRIX4X4 matRotate;

    matRotate.M(0,0) = (1.0f - 2.0f*q.y*q.y - 2.0f*q.z*q.z);
    matRotate.M(0,1) = (       2.0f*q.x*q.y - 2.0f*q.z*q.w);
    matRotate.M(0,2) =+(       2.0f*q.x*q.z + 2.0f*q.y*q.w);
    matRotate.M(0,3) = (0.0f);

    matRotate.M(1,0) = (       2.0f*q.x*q.y + 2.0f*q.z*q.w);
    matRotate.M(1,1) = (1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z);
    matRotate.M(1,2) =+(       2.0f*q.z*q.y - 2.0f*q.x*q.w);
    matRotate.M(1,3) = (0.0f);

    matRotate.M(2,0) =+(       2.0f*q.x*q.z - 2.0f*q.y*q.w);
    matRotate.M(2,1) =+(       2.0f*q.z*q.y + 2.0f*q.x*q.w);
    matRotate.M(2,2) = (1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y);
    matRotate.M(2,3) = (0.0f);

    matRotate.M(3,0) = 0.0f;
    matRotate.M(3,1) = 0.0f;
    matRotate.M(3,2) = 0.0f;
    matRotate.M(3,3) = 1.0f;

    return matRotate;
}


//--------------------------------------------------------------------------------------
// Name: FrmMatrixReflect()
// Desc: Computes a matrix reflected about a plane
//--------------------------------------------------------------------------------------
FRMMATRIX4X4 FrmMatrixReflect( FRMMATRIX4X4& matSrcMatrix, FRMVECTOR3& n, FLOAT32 d )
{
    FRMVECTOR3 x = *(FRMVECTOR3*)&matSrcMatrix.m[0][0];
    FRMVECTOR3 y = *(FRMVECTOR3*)&matSrcMatrix.m[1][0];
    FRMVECTOR3 z = *(FRMVECTOR3*)&matSrcMatrix.m[2][0];
    FRMVECTOR3 t = *(FRMVECTOR3*)&matSrcMatrix.m[3][0];

    // Mirror translation
    FRMVECTOR3 mt = t - 2 * n * ( FrmVector3Dot( t, n ) - d );

    // Mirror XYZ rotation
    FRMVECTOR3 xt = x + t;
    FRMVECTOR3 yt = y + t;
    FRMVECTOR3 zt = z + t;
    FRMVECTOR3 mx = x + t - 2 * n * ( FrmVector3Dot( xt, n ) - d ) - mt;
    FRMVECTOR3 my = y + t - 2 * n * ( FrmVector3Dot( yt, n ) - d ) - mt;
    FRMVECTOR3 mz = z + t - 2 * n * ( FrmVector3Dot( zt, n ) - d ) - mt;

    FRMMATRIX4X4 matResult;
    matResult.m[0][0] = mx.x;
    matResult.m[0][1] = mx.y;
    matResult.m[0][2] = mx.z;
    matResult.m[0][3] = 0;

    matResult.m[1][0] = my.x;
    matResult.m[1][1] = my.y;
    matResult.m[1][2] = my.z;
    matResult.m[1][3] = 0;

    matResult.m[2][0] = mz.x;
    matResult.m[2][1] = mz.y;
    matResult.m[2][2] = mz.z;
    matResult.m[2][3] = 0;

    matResult.m[3][0] = mt.x;
    matResult.m[3][1] = mt.y;
    matResult.m[3][2] = mt.z;
    matResult.m[3][3] = 1;

    return matResult;
}

