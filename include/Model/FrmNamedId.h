// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_VERTEX_SEMANTIC_H
#define ADRENO_VERTEX_SEMANTIC_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct NamedId
    {
        static const int MAX_NAME_LENGTH = 32;

        char  Name[ MAX_NAME_LENGTH ];
        INT32 Id;
        
        NamedId();
        NamedId( const char* name );
        NamedId& operator = ( const char* name );
        
        void SetName( const char* name );
        
        bool operator == ( const NamedId& other );
        bool operator != ( const NamedId& other );
    };
}

//-----------------------------------------------------------------------------

#endif