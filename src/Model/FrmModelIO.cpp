// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmModelIO.h"
#include "FrmFile.h"
#include "FrmMemoryBuffer.h"
#include "FrmPointerTable.h"
#include <vector>

//-----------------------------------------------------------------------------

namespace
{
    #ifndef UNDER_CE
        typedef FRM_FILE AdrenoFile;
    #else
        typedef FILE AdrenoFile;
    #endif

    //-----------------------------------------------------------------------------

    const UINT32 FRM_MODEL_MAGIC_ID = ( ('M'<<0L) | ('D'<<8L) | ('L'<<16) | (1<<24L) );
    const UINT32 FRM_ANIM_MAGIC_ID  = ( ('A'<<0L) | ('N'<<8L) | ('M'<<16) | (1<<24L) );

    //-----------------------------------------------------------------------------

    struct FRM_MODEL_FILE_HEADER
    {
        UINT32 Magic;
        UINT32 DataSize;
        
        FRM_MODEL_FILE_HEADER()
        : Magic( FRM_MODEL_MAGIC_ID )
        , DataSize( 0 )
        {
        
        }
    };
    
    //-----------------------------------------------------------------------------

    struct FRM_ANIM_FILE_HEADER
    {
        UINT32 Magic;
        UINT32 DataSize;
        
        FRM_ANIM_FILE_HEADER()
        : Magic( FRM_ANIM_MAGIC_ID )
        , DataSize( 0 )
        {
        
        }
    };

    //-----------------------------------------------------------------------------

    typedef std::vector< Adreno::PointerFixup > PointerFixupArray;

    //-----------------------------------------------------------------------------

    size_t GetMemberOffset( const void* owner_address, const void* member_address )
    {
        size_t owner_position  = (size_t)owner_address;
        size_t member_position = (size_t)member_address;
        size_t member_offset   = member_position - owner_position;

        return member_offset;
    }

    //-----------------------------------------------------------------------------

    template< typename T >
    void WriteModelValue( T value, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        INT32 value_size = sizeof( value );
        data.WriteData( data_offset, &value, value_size );
    }

    //-----------------------------------------------------------------------------

    template< typename T >
    void WriteModelValueArray( int num_array_values, const T* array_values, INT32 array_member_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        INT32 array_values_data_offset = Adreno::FRM_NULL_POINTER_OFFSET;

        // Write the array values
        if( array_values )
        {
            // Get memory for the array of values
            INT32 value_data_size  = sizeof( array_values[ 0 ] );
            INT32 values_data_size = num_array_values * value_data_size;
            
            array_values_data_offset = data.ExpandBuffer( values_data_size );
            
            // Write each value
            for( int i = 0; i < num_array_values; ++i )
            {
                const T& value             = array_values[ i ];
                INT32    value_data_offset = array_values_data_offset + ( i * value_data_size );

                WriteModelValue( value, value_data_offset, data, pointer_fixups );
            }
        }

        // Add a pointer fixup for the array member
        Adreno::PointerFixup array_pointer( array_member_offset, array_values_data_offset );
        pointer_fixups.push_back( array_pointer );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::VertexProperty& vertex_property, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Usage
        INT32 usage_data_offset = data_offset + GetMemberOffset( &vertex_property, &vertex_property.Usage );
        WriteModelValue( vertex_property.Usage, usage_data_offset, data, pointer_fixups );

        // Type
        INT32 type_data_offset = data_offset + GetMemberOffset( &vertex_property, &vertex_property.Type );
        WriteModelValue( vertex_property.Type, type_data_offset, data, pointer_fixups );

        // Offset
        INT32 offset_data_offset = data_offset + GetMemberOffset( &vertex_property, &vertex_property.Offset );
        WriteModelValue( vertex_property.Offset, offset_data_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::VertexFormat& vertex_format, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Stride
        INT32 stride_data_offset = data_offset + GetMemberOffset( &vertex_format, &vertex_format.Stride );
        WriteModelValue( vertex_format.Stride, stride_data_offset, data, pointer_fixups );

        // NumProperties
        INT32 num_properties_data_offset = data_offset + GetMemberOffset( &vertex_format, &vertex_format.NumProperties );
        WriteModelValue( vertex_format.NumProperties, num_properties_data_offset, data, pointer_fixups );

        // Properties
        INT32 properties_member_offset = data_offset + GetMemberOffset( &vertex_format, &vertex_format.Properties );
        WriteModelValueArray( vertex_format.NumProperties, vertex_format.Properties, properties_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::VertexBuffer& vertex_buffer, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Format
        INT32 format_data_offset = data_offset + GetMemberOffset( &vertex_buffer, &vertex_buffer.Format );
        WriteModelValue( vertex_buffer.Format, format_data_offset, data, pointer_fixups );

        // NumVerts
        INT32 num_verts_data_offset = data_offset + GetMemberOffset( &vertex_buffer, &vertex_buffer.NumVerts );
        WriteModelValue( vertex_buffer.NumVerts, num_verts_data_offset, data, pointer_fixups );

        // BufferSize
        INT32 buffer_size_data_offset = data_offset + GetMemberOffset( &vertex_buffer, &vertex_buffer.BufferSize );
        WriteModelValue( vertex_buffer.BufferSize, buffer_size_data_offset, data, pointer_fixups );

        // Buffer
        INT32 buffer_member_offset = data_offset + GetMemberOffset( &vertex_buffer, &vertex_buffer.Buffer );
        WriteModelValueArray( vertex_buffer.BufferSize, vertex_buffer.Buffer, buffer_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::IndexBuffer& index_buffer, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // NumIndices
        INT32 num_indices_data_offset = data_offset + GetMemberOffset( &index_buffer, &index_buffer.NumIndices );
        WriteModelValue( index_buffer.NumIndices, num_indices_data_offset, data, pointer_fixups );

        // Indices
        INT32 indices_member_offset = data_offset + GetMemberOffset( &index_buffer, &index_buffer.Indices );
        WriteModelValueArray( index_buffer.NumIndices, index_buffer.Indices, indices_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::MeshSurfaceArray& surface_array, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // NumSurfaces
        INT32 num_surfaces_data_offset = data_offset + GetMemberOffset( &surface_array, &surface_array.NumSurfaces );
        WriteModelValue( surface_array.NumSurfaces, num_surfaces_data_offset, data, pointer_fixups );

        // Surfaces
        INT32 surfaces_member_offset = data_offset + GetMemberOffset( &surface_array, &surface_array.Surfaces );
        WriteModelValueArray( surface_array.NumSurfaces, surface_array.Surfaces, surfaces_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::Mesh& mesh, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Vertices
        INT32 vertices_data_offset = data_offset + GetMemberOffset( &mesh, &mesh.Vertices );
        WriteModelValue( mesh.Vertices, vertices_data_offset, data, pointer_fixups );

        // Indices
        INT32 indices_data_offset = data_offset + GetMemberOffset( &mesh, &mesh.Indices );
        WriteModelValue( mesh.Indices, indices_data_offset, data, pointer_fixups );

        // Surfaces
        INT32 surfaces_data_offset = data_offset + GetMemberOffset( &mesh, &mesh.Surfaces );
        WriteModelValue( mesh.Surfaces, surfaces_data_offset, data, pointer_fixups );

        // JointIndex
        INT32 joint_index_data_offset = data_offset + GetMemberOffset( &mesh, &mesh.JointIndex );
        WriteModelValue( mesh.JointIndex, joint_index_data_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::Model& model, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // NumMaterials
        INT32 num_materials_data_offset = data_offset + GetMemberOffset( &model, &model.NumMaterials );
        WriteModelValue( model.NumMaterials, num_materials_data_offset, data, pointer_fixups );

        // Materials
        INT32 materials_member_offset = data_offset + GetMemberOffset( &model, &model.Materials );
        WriteModelValueArray( model.NumMaterials, model.Materials, materials_member_offset, data, pointer_fixups );

        // NumMeshes
        INT32 num_meshes_data_offset = data_offset + GetMemberOffset( &model, &model.NumMeshes );
        WriteModelValue( model.NumMeshes, num_meshes_data_offset, data, pointer_fixups );

        // Meshes
        INT32 meshes_member_offset = data_offset + GetMemberOffset( &model, &model.Meshes );
        WriteModelValueArray( model.NumMeshes, model.Meshes, meshes_member_offset, data, pointer_fixups );

        // NumJoints
        INT32 num_joints_data_offset = data_offset + GetMemberOffset( &model, &model.NumJoints );
        WriteModelValue( model.NumJoints, num_joints_data_offset, data, pointer_fixups );

        // Joints
        INT32 joints_member_offset = data_offset + GetMemberOffset( &model, &model.Joints );
        WriteModelValueArray( model.NumJoints, model.Joints, joints_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelData( const Adreno::Model& model, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Get memory for the model
        INT32 model_data_size   = sizeof( model );
        INT32 model_data_offset = data.ExpandBuffer( model_data_size );
        
        WriteModelValue( model, model_data_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::AnimationTrack& track, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Id
        INT32 id_data_offset = data_offset + GetMemberOffset( &track, &track.Id );
        WriteModelValue( track.Id, id_data_offset, data, pointer_fixups );

        // NumKeyframes
        INT32 num_keyframes_data_offset = data_offset + GetMemberOffset( &track, &track.NumKeyframes );
        WriteModelValue( track.NumKeyframes, num_keyframes_data_offset, data, pointer_fixups );

        // Keyframes
        INT32 keyframes_member_offset = data_offset + GetMemberOffset( &track, &track.Keyframes );
        WriteModelValueArray( track.NumKeyframes, track.Keyframes, keyframes_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteModelValue( const Adreno::Animation& animation, INT32 data_offset, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // NumFrames
        INT32 num_frames_data_offset = data_offset + GetMemberOffset( &animation, &animation.NumFrames );
        WriteModelValue( animation.NumFrames, num_frames_data_offset, data, pointer_fixups );

        // NumTracks
        INT32 num_tracks_data_offset = data_offset + GetMemberOffset( &animation, &animation.NumTracks );
        WriteModelValue( animation.NumTracks, num_tracks_data_offset, data, pointer_fixups );

        // Tracks
        INT32 tracks_member_offset = data_offset + GetMemberOffset( &animation, &animation.Tracks );
        WriteModelValueArray( animation.NumTracks, animation.Tracks, tracks_member_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WriteAnimationData( const Adreno::Animation& animation, Adreno::MemoryBuffer& data, PointerFixupArray& pointer_fixups )
    {
        // Get memory for the animation
        INT32 anim_data_size   = sizeof( animation );
        INT32 anim_data_offset = data.ExpandBuffer( anim_data_size );
        
        WriteModelValue( animation, anim_data_offset, data, pointer_fixups );
    }

    //-----------------------------------------------------------------------------

    void WritePointerFixupTable( AdrenoFile* file, const PointerFixupArray& pointer_fixups )
    {
        // Write pointer table
        int num_pointers        = (int)( pointer_fixups.size() );
        int pointer_fixups_size = num_pointers * sizeof( Adreno::PointerFixup );

        Adreno::PointerTable pointer_table;
        pointer_table.NumPointers = num_pointers;

        FrmFile_Write( file, &pointer_table, sizeof( pointer_table ) );
        
        if( pointer_fixups_size > 0 )
        {
            FrmFile_Write( file, (void*)&(pointer_fixups[ 0 ]), pointer_fixups_size );
        }
    }

    //-----------------------------------------------------------------------------

    void ReadPointerFixupTable( AdrenoFile* file, Adreno::PointerTable& pointer_table )
    {
        pointer_table = Adreno::PointerTable();
        if( !file ) { return; }

        FrmFile_Read( file, &pointer_table, sizeof( pointer_table ) );
        pointer_table.PointerFixups = NULL;
        
        int num_pointers = pointer_table.NumPointers;
        
        if( num_pointers > 0 )
        {
            pointer_table.PointerFixups = new Adreno::PointerFixup[ num_pointers ];

            UINT32 pointer_fixups_size = num_pointers * sizeof( Adreno::PointerFixup );
            FrmFile_Read( file, pointer_table.PointerFixups, pointer_fixups_size );
        }
    }

    //-----------------------------------------------------------------------------

    void WriteDataBuffer( AdrenoFile* file, const Adreno::MemoryBuffer& data )
    {
        const BYTE* data_buffer = data.GetBuffer();
        INT32       data_size   = data.BufferSize();

        FrmFile_Write( file, (void*)data_buffer, data_size );
    }

    //-----------------------------------------------------------------------------

    void FixupPointers( BYTE* data_buffer, const Adreno::PointerTable& pointer_table )
    {
        if( !data_buffer ) { return; }

        for( int i = 0; i < pointer_table.NumPointers; ++i )
        {
            const Adreno::PointerFixup& pointer_fixup = pointer_table.PointerFixups[ i ];

            size_t* pointer = (size_t*)( data_buffer + pointer_fixup.PointerOffset );
            
            if( pointer_fixup.PointeeOffset == Adreno::FRM_NULL_POINTER_OFFSET )
            {
                *pointer = NULL;
            }
            else
            {
                size_t pointee = (size_t)( data_buffer + pointer_fixup.PointeeOffset );
                *pointer = pointee;
            }
        }
    }
}

//-----------------------------------------------------------------------------

BOOL Adreno::FrmWriteModelToFile( const Adreno::Model& model, const CHAR* filename )
{
    AdrenoFile* file;

    if( !FrmFile_Open( filename, FRM_FILE_WRITE, &file ) )
    { return FALSE; }

    // Create data stream and pointer fixup table
    Adreno::MemoryBuffer data;
    PointerFixupArray    pointer_fixups;

    WriteModelData( model, data, pointer_fixups );

    // Write header
    FRM_MODEL_FILE_HEADER header;
    header.DataSize = data.BufferSize();

    FrmFile_Write( file, &header, sizeof( header ) );

    // Write pointer table and data
    WritePointerFixupTable( file, pointer_fixups );
    WriteDataBuffer( file, data );

    FrmFile_Close( file );
    return TRUE;
}

//-----------------------------------------------------------------------------

Adreno::Model* Adreno::FrmLoadModelFromFile( const CHAR* filename )
{
    AdrenoFile* file;

    if( !FrmFile_Open( filename, FRM_FILE_READ, &file ) )
    { return NULL; }

    // Read header
    FRM_MODEL_FILE_HEADER header;
    FrmFile_Read( file, &header, sizeof( header ) );

    if( ( header.Magic != FRM_MODEL_MAGIC_ID ) || ( header.DataSize == 0 ) )
    {
        FrmFile_Close( file );
        return NULL;
    }

    // Read pointer fixup table
    Adreno::PointerTable pointer_table;
    ReadPointerFixupTable( file, pointer_table );

    // Read data
    BYTE* data_buffer = new BYTE[ header.DataSize ];
    FrmFile_Read( file, data_buffer, header.DataSize );
    FrmFile_Close( file );

    // Fixup pointers
    FixupPointers( data_buffer, pointer_table );
    Adreno::Model* model = (Adreno::Model*)data_buffer;

    return model;
}

//-----------------------------------------------------------------------------

void Adreno::FrmDestroyLoadedModel( Adreno::Model*& model )
{
    if( model )
    {
        BYTE* model_memory = (BYTE*)model;
        delete[] model_memory;
        model = NULL;
    }
}

//-----------------------------------------------------------------------------

BOOL Adreno::FrmWriteAnimationToFile( const Adreno::Animation& animation, const CHAR* filename )
{
    AdrenoFile* file;

    if( !FrmFile_Open( filename, FRM_FILE_WRITE, &file ) )
    { return FALSE; }

    // Create data stream and pointer fixup table
    Adreno::MemoryBuffer data;
    PointerFixupArray    pointer_fixups;

    WriteAnimationData( animation, data, pointer_fixups );

    // Write header
    FRM_ANIM_FILE_HEADER header;
    header.DataSize = data.BufferSize();

    FrmFile_Write( file, &header, sizeof( header ) );

    // Write pointer table and data
    WritePointerFixupTable( file, pointer_fixups );
    WriteDataBuffer( file, data );

    FrmFile_Close( file );
    return TRUE;
}

//-----------------------------------------------------------------------------

Adreno::Animation* Adreno::FrmLoadAnimationFromFile( const CHAR* filename )
{
    AdrenoFile* file;

    if( !FrmFile_Open( filename, FRM_FILE_READ, &file ) )
    { return NULL; }

    // Read header
    FRM_ANIM_FILE_HEADER header;
    FrmFile_Read( file, &header, sizeof( header ) );

    if( ( header.Magic != FRM_ANIM_MAGIC_ID ) || ( header.DataSize == 0 ) )
    {
        FrmFile_Close( file );
        return NULL;
    }

    // Read pointer fixup table
    Adreno::PointerTable pointer_table;
    ReadPointerFixupTable( file, pointer_table );

    // Read data
    BYTE* data_buffer = new BYTE[ header.DataSize ];
    FrmFile_Read( file, data_buffer, header.DataSize );
    FrmFile_Close( file );

    // Fixup pointers
    FixupPointers( data_buffer, pointer_table );
    Adreno::Animation* animation = (Adreno::Animation*)data_buffer;

    return animation;
}

//-----------------------------------------------------------------------------

void Adreno::FrmDestroyLoadedAnimation( Adreno::Animation*& animation )
{
    if( animation )
    {
        BYTE* animation_memory = (BYTE*)animation;
        delete[] animation_memory;
        animation = NULL;
    }
}
