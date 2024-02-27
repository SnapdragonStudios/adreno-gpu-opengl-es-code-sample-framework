//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_VERTEX_PROPERTY_H
#define ADRENO_VERTEX_PROPERTY_H

#include <string>
#include "FrmNamedId.h"
#include "FrmPlatform.h"
#include "FrmUtils.h"
#include "FrmVertexPropertyType.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct VertexProperty
    {
        NamedId            Usage;
        VertexPropertyType Type;
        UINT32             Offset;
        
        VertexProperty();

        UINT32  Size         () const;
        UINT32  NumValues    () const;
        bool    IsNormalized () const;
        bool    Valid() const;
        inline bool operator==(const VertexProperty& rhs)
        {
            return Usage == rhs.Usage && Type == rhs.Type && Offset == rhs.Offset;
        }
    };

    // glTF supports : POSITION, NORMAL, TANGENT, TEXCOORD_0, TEXCOORD_1, COLOR_0, JOINTS_0, and WEIGHTS_0
    enum GltfAttribs
    {
        ATTRIB_FIRST_VALUE = 0,
        ATTRIB_INDICES = ATTRIB_FIRST_VALUE,
        ATTRIB_POSITION,
        ATTRIB_NORMAL,
        ATTRIB_TANGENT,
        ATTRIB_TEXCOORD_0,
        ATTRIB_TEXCOORD_1,
        ATTRIB_COLOR_0,
        ATTRIB_JOINTS_0,
        ATTRIB_WEIGHTS_0,
        NUM_GLTF_ATTRIBS
    };
    inline void AssertValid(const enum GltfAttribs e)
    {
        ADRENO_ASSERT(e >= ATTRIB_FIRST_VALUE, __FILE__, __LINE__);
        ADRENO_ASSERT(e < NUM_GLTF_ATTRIBS, __FILE__, __LINE__);
    }
    const char* const s_gltfAttribsString[] =
    {
        "INDICES_dummy",///<gltf doesn't use this string; the accessor index where for the index buffer information resides isn't parsed; it's supplied directly in a Primitive
        "POSITION",
        "NORMAL",
        "TANGENT",
        "TEXCOORD_0",
        "TEXCOORD_1",
        "COLOR_0",
        "JOINTS_0",
        "WEIGHTS_0",
    };

    inline bool VertexPropertyIs(const char* const vertexPropertyUsageName, const Adreno::GltfAttribs gltfAttrib)
    {
        ADRENO_ASSERT(CStringNotEmpty(vertexPropertyUsageName), __FILE__, __LINE__);
        Adreno::AssertValid(gltfAttrib);

        return std::strncmp(vertexPropertyUsageName, Adreno::s_gltfAttribsString[gltfAttrib], Adreno::NamedId::MAX_NAME_LENGTH) == 0;
    }
}

//-----------------------------------------------------------------------------

#endif