// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmMemoryBuffer.h"

//-----------------------------------------------------------------------------

Adreno::MemoryBuffer::MemoryBuffer()
: m_size( 0 )
, m_buffer( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::MemoryBuffer::~MemoryBuffer()
{
    DeleteBuffer();
}

//-----------------------------------------------------------------------------

INT32 Adreno::MemoryBuffer::BufferSize() const
{
    return m_size;
}

//-----------------------------------------------------------------------------

BYTE* Adreno::MemoryBuffer::GetBuffer()
{
    return m_buffer;
}

//-----------------------------------------------------------------------------

const BYTE* Adreno::MemoryBuffer::GetBuffer() const
{
    return m_buffer;
}

//-----------------------------------------------------------------------------

INT32 Adreno::MemoryBuffer::ExpandBuffer( INT32 num_new_bytes )
{
    INT32 old_size = BufferSize();
    INT32 new_size = old_size + num_new_bytes;

    if( new_size > 0 )
    {
        BYTE* new_buffer = new BYTE[ new_size ];

        // Copy the old buffer
        if( m_buffer )
        {
            for( INT32 i = 0; i < old_size; ++i )
            {
                new_buffer[ i ] = m_buffer[ i ];
            }

            DeleteBuffer();
        }

        // Zero out the new bytes
        for( INT32 i = old_size; i < new_size; ++i )
        {
            new_buffer[ i ] = 0;
        }

        // Use the new buffer
        m_size   = new_size;
        m_buffer = new_buffer;
    }

    return old_size;
}

//-----------------------------------------------------------------------------

void Adreno::MemoryBuffer::WriteData( INT32 start_position, const void* data, INT32 size )
{
    if( data && size > 0 )
    {
        if( ( start_position >= 0 ) && ( start_position + size <= BufferSize() ) )
        {
            const BYTE* source_data = (const BYTE*)data;

            for( INT32 i = 0; i < size; ++i )
            {
                BYTE& dest = m_buffer[ start_position + i ];
                dest = source_data[ i ];
            }
        }
    }
}

//-----------------------------------------------------------------------------

void Adreno::MemoryBuffer::DeleteBuffer()
{
    if( m_buffer )
    {
        delete[] m_buffer;
        m_buffer = NULL;
    }
}
