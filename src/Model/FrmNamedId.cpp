// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmNamedId.h"
#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

Adreno::NamedId::NamedId()
: Id( 0 )
{
    Name[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------

Adreno::NamedId::NamedId( const char* name )
: Id( 0 )
{
    SetName( name );
}

//-----------------------------------------------------------------------------

Adreno::NamedId& Adreno::NamedId::operator = ( const char* name )
{
    SetName( name );
    return *this;
}

//-----------------------------------------------------------------------------

void Adreno::NamedId::SetName( const char* name )
{
    FrmStrncpy( Name, name, Adreno::NamedId::MAX_NAME_LENGTH );
    // TODO: Hash name to create id
}

//-----------------------------------------------------------------------------

bool Adreno::NamedId::operator == ( const NamedId& other )
{
    // TODO: Just compare the hashed ids once we implement the hashing function
    bool is_equal = FrmStrcmp( Name, other.Name ) == 0;
    return is_equal;
}

//-----------------------------------------------------------------------------

bool Adreno::NamedId::operator != ( const NamedId& other )
{
    // TODO: Just compare the hashed ids once we implement the hashing function
    bool is_not_equal = FrmStrcmp( Name, other.Name ) != 0;
    return is_not_equal;
}
