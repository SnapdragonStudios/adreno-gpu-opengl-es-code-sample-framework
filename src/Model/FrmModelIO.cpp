//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmModelIO.h"
#include "FrmFile.h"
#include "FrmMemoryBuffer.h"
#include "FrmPointerTable.h"
#include "FrmStdLib.h"
#include "FrmResourceGLES.h"
#include "FrmVertexBuffer.h"
#include <ktx.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch" //tinygltf submodule triggers this warning (harmlessly, so far as I've seen); it's presently not practical to try to push a fix to the submodule
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_STB_IMAGE_WRIE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define JSON_NOEXCEPTION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#include "tinygltf/tiny_gltf.h"
#pragma GCC diagnostic pop

extern AAssetManager* g_pAssetManager;

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
    const UINT32 FRM_ANIM_MAGIC_ID  = ( ('a'<<0L) | ('N'<<8L) | ('M'<<16) | (1<<24L) );

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
                *pointer = static_cast<size_t>(NULL);
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
static bool Gltf_FileExists(const std::string& abs_filename, void* vAssetManager)
{
    return true;
}
static std::string Gltf_ExpandFilePath(const std::string& filename, void* vAssetManager)
{
    return filename;
}
static bool Gltf_ReadWholeFile(std::vector<unsigned char>* output, std::string* error, const std::string& filename, void*)
{
    ADRENO_ASSERT(output, __FILE__, __LINE__);
    ADRENO_ASSERT(error, __FILE__, __LINE__);
    ADRENO_ASSERT(filename.size() > 0, __FILE__, __LINE__);

    ADRENO_ASSERT(g_pAssetManager, __FILE__, __LINE__);
    tinygltf::asset_manager = g_pAssetManager;
    const bool readWholeFileResult = tinygltf::ReadWholeFile(output, error, filename, NULL);
    if(!readWholeFileResult)
    {
        LOGE(   "%s:%i:g_pAssetManager=%p, error=%s, filename=%s, readWholeFileResult=%i", 
                __FILE__, __LINE__, g_pAssetManager, error->c_str(), filename.c_str(), readWholeFileResult);
    }    
    ADRENO_ASSERT(readWholeFileResult, __FILE__, __LINE__);

    return readWholeFileResult;
}
///provided as a convenience for reading in whole datafiles from Android's asset system
bool Adreno::tinygltf_ReadWholeFile(std::vector<unsigned char>* const output, std::string* const error, const std::string& filename)
{
    return Gltf_ReadWholeFile(output, error, filename, NULL);
}
static bool Gltf_WriteWholeFile(std::string*, const std::string&, const std::vector<unsigned char>&, void*)
{
    return false;
}
bool StubGltfLoadImageDataFunction(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
    return true;
}
//-----------------------------------------------------------------------------

//BEG_#remove
typedef struct _gltfAttribInfo
{
    // Index into the accessor list
    int32_t         accessorIndx = -1;

    // Number of elements
    uint32_t        count = 0;

    // Number of bytes per element (span)
    uint32_t        bytesPerElem = 0;

    // Total number of bytes
    size_t          bytesTotal = 0;

    // Pointer to data within the glTF buffer
    void* pData = nullptr;

} gltfAttribInfo;
//END_#remove

//BEG_#remove
/// Describes the (platform agnostic) format and layout of a single vertex.
/// Can be constructed at compile time (eg to describe a known hard-coded layout vertex/structure) or populated from data (eg ShaderDescriptionLoader)
/// @ingroup Material
class VertexFormat
{
public:
    struct Element
    {
        class ElementType
        {
        public:
            enum class t {
                Int32,
                Float,
                Vec2,
                Vec3,
                Vec4
            };
            /*constexpr C++14*/ ElementType(const t _type) : type(_type) {}
            /*constexpr C++14*/ operator t() const { return type; }
            /*constexpr C++14*/ int size() const {
                switch (type) {
                default:
                case t::Int32:
                    return 4;
                case t::Float:
                    return 4;
                case t::Vec2:
                    return 8;
                case t::Vec3:
                    return 12;
                case t::Vec4:
                    return 16;
                }
            }
        private:
            const t type;
        };
        uint32_t offset;
        ElementType type;
        bool operator==(const Element& other) const { return offset == other.offset && type == other.type; };
    };
    enum class eInputRate {
        Vertex, Instance
    };
    const uint32_t span;                        ///< span of this vertex in bytes
    const eInputRate inputRate;                 ///< input rate of this vertex (Vertex or Instance rate)
    const std::vector<Element> elements;
    const std::vector<std::string> elementIds;

    const uint32_t& GetUint32(uint8_t* src, uint32_t elementIdx) const
    {
        if (elements[elementIdx].type == Element::ElementType::t::Int32)
        {
            return *((uint32_t*)(src + elements[elementIdx].offset));
        }
        ADRENO_ASSERT(false, __FILE__, __LINE__);
        return *((uint32_t*)src);
    }
};

typedef VertexFormat::Element::ElementType::t VertexElementType;

struct vertex_layout
{
    float pos[3];           // SHADER_ATTRIB_LOC_POSITION
    float normal[3];        // SHADER_ATTRIB_LOC_NORMAL
    float uv[2];            // SHADER_ATTRIB_LOC_TEXCOORD0
    float color[4];         // SHADER_ATTRIB_LOC_COLOR
    float tangent[3];       // SHADER_ATTRIB_LOC_TANGENT
    // float binormal[3];      // SHADER_ATTRIB_LOC_BITANGENT
    static VertexFormat sFormat;
};
//END_#remove

static int VertexPropertyGltfStride(
    const tinygltf::Accessor& accessorData, 
    const tinygltf::BufferView& viewData, 
    const char* const gltfVertexAttributeString,
    const size_t gltfVertexAttributeIndex)
{
    ADRENO_ASSERT(CStringNotEmpty(gltfVertexAttributeString), __FILE__, __LINE__);

    const int stride = accessorData.ByteStride(viewData);
    if (stride < 0)
    {
        LOGE("Error loading: Cannot calculate data stride for gltfVertexAttribute=%zu/%s", gltfVertexAttributeIndex, gltfVertexAttributeString);
        ADRENO_ASSERT(false, __FILE__, __LINE__);
        return false;
    }
    //  LOGI("stride=%i read for gltfVertexAttributeIndex=%zu/%s", stride, gltfVertexAttributeIndex, gltfVertexAttributeString);
    return stride;
}
static bool GltfParseVertexProperty(
    Adreno::VertexFormat*const vertexFormatPtr,
    const tinygltf::Model& gltfModel,
    const int accessorIndx,
    const char* const gltfVertexAttributeString,
    const Adreno::GltfAttribs gltfVertexAttributeIndex)
{
    ADRENO_REF(vertexFormatPtr, vertexFormat);
    ADRENO_ASSERT(CStringNotEmpty(gltfVertexAttributeString), __FILE__, __LINE__);
    Adreno::AssertValid(static_cast<const enum Adreno::GltfAttribs>(gltfVertexAttributeIndex));

    const tinygltf::Accessor& accessorData = gltfModel.accessors[accessorIndx];
    const tinygltf::BufferView& viewData = gltfModel.bufferViews[accessorData.bufferView];
    Adreno::VertexProperty vertexProperty;
    vertexProperty.Usage.SetName(gltfVertexAttributeString);

    switch (gltfVertexAttributeIndex)
    {
        case Adreno::ATTRIB_INDICES:
        {
            return true;//not stored as an interleaved vertex attribute; stored in separate index buffer
        }
        case Adreno::ATTRIB_POSITION:
        case Adreno::ATTRIB_NORMAL:
        {
            vertexProperty.Type = Adreno::FLOAT3_TYPE;
            break;
        }
        case Adreno::ATTRIB_TEXCOORD_0:
        case Adreno::ATTRIB_TEXCOORD_1:
        {
            vertexProperty.Type = Adreno::FLOAT2_TYPE;
            break;
        }
        case Adreno::ATTRIB_TANGENT:
        case Adreno::ATTRIB_COLOR_0:
        {
            vertexProperty.Type = Adreno::FLOAT4_TYPE;
            break;
        }
        case Adreno::ATTRIB_JOINTS_0:
        case Adreno::ATTRIB_WEIGHTS_0:
        default:
        {
            LOGE("gltfVertexAttributeIndex=%i/%s not supported", gltfVertexAttributeIndex, gltfVertexAttributeString);
            ADRENO_ASSERT(false, __FILE__, __LINE__);
            return false;
        }
    }
    //LOGI("vertexProperty.Size()=%u, VertexPropertyGltfStride(%s)=%i", 
    //      vertexProperty.Size(), gltfVertexAttributeString, VertexPropertyGltfStride(accessorData, viewData, gltfVertexAttributeString, gltfVertexAttributeIndex));
    ADRENO_ASSERT( vertexProperty.Size() == VertexPropertyGltfStride(accessorData, viewData, gltfVertexAttributeString, gltfVertexAttributeIndex), 
                __FILE__, __LINE__);//verify VertexProperty agrees with stride (eg bytes-to-type mapping is consistent with type-to-bytes mapping)
    vertexFormat.SetProperty(gltfVertexAttributeIndex, vertexProperty);
    //LOGI(   "%s:%i:gltfVertexAttributeIndex=%i; vertexProperty.Size()=%u; vertexProperty.Usage.Name=%s",
    //        __FILE__, __LINE__, gltfVertexAttributeIndex, vertexProperty.Size(), vertexProperty.Usage.Name);
    
    return true;
}

static void VertexBufferEnsureSpace(Adreno::Mesh* const adrenoMeshPtr, const int accessorDataCount)
{
    ADRENO_REF(adrenoMeshPtr, adrenoMesh);
    assert(accessorDataCount > 0);

    Adreno::VertexBuffer& vertexBuffer = adrenoMesh.Vertices;
    if (vertexBuffer.NumVerts == 0)
    {
        vertexBuffer.InitializeBuffer(accessorDataCount);
    }
    else
    {
        ADRENO_ASSERT(vertexBuffer.NumVerts == accessorDataCount, __FILE__, __LINE__);
    }
    ADRENO_ASSERT(vertexBuffer.NumVerts >= 0, __FILE__, __LINE__);
    ADRENO_ASSERT(vertexBuffer.NumVerts <= adrenoMesh.Indices.NumIndices, __FILE__, __LINE__);
}
static bool GltfParseMesh(
    Adreno::Mesh* const adrenoMeshPtr,
    const tinygltf::Model& gltfModel,
    const int accessorIndx,
    const char* const gltfVertexAttributeString,
    const size_t gltfVertexAttributeIndex)
{
    ADRENO_REF(adrenoMeshPtr, adrenoMesh);

    //LOGI(   "%s:%i:gltfVertexAttributeIndex=%zu; adrenoMesh.Indices.NumIndices=%u; adrenoMesh.Indices.Indices=%p",
    //        __FILE__, __LINE__, gltfVertexAttributeIndex, adrenoMesh.Indices.NumIndices, adrenoMesh.Indices.Indices);

    //read data blobs that conform to vertex attribute format
    const tinygltf::Accessor& accessorData = gltfModel.accessors[accessorIndx];
    const tinygltf::BufferView& viewData = gltfModel.bufferViews[accessorData.bufferView];
    const tinygltf::Buffer& bufferData = gltfModel.buffers[viewData.buffer];
    //LOGI("%s:%i:accessorIndx=%i, gltfModel.buffers.size()=%zu, viewData.buffer=%i bufferData.data.at(0)=%p, viewData.byteOffset=%zu, accessorData.byteOffset=%zu, viewData.byteOffset + accessorData.byteOffset=%zu", 
    //    __FILE__, __LINE__, accessorIndx, gltfModel.buffers.size(), viewData.buffer, &bufferData.data.at(0), viewData.byteOffset, accessorData.byteOffset, viewData.byteOffset + accessorData.byteOffset);

    const UINT8* dataPtr = reinterpret_cast<const UINT8*>(&bufferData.data.at(viewData.byteOffset + accessorData.byteOffset));
    //LOGI("%s:%i:dataPtr=%p", __FILE__, __LINE__, dataPtr);

    const int gltfStride = VertexPropertyGltfStride(accessorData, viewData, gltfVertexAttributeString, gltfVertexAttributeIndex);
    Adreno::IndexBuffer& indexBuffer = adrenoMesh.Indices;
    const Adreno::VertexFormat& vertexFormat = adrenoMesh.Vertices.Format;
    switch (gltfVertexAttributeIndex)
    {
        case Adreno::ATTRIB_INDICES:
        {
            adrenoMesh.Indices.Resize(accessorData.count);
            ADRENO_ASSERT(indexBuffer.NumIndices >= 3, __FILE__, __LINE__);
            if (gltfStride < 0)
            {
                ADRENO_ASSERT(false, __FILE__, __LINE__);
                return false;
            }

            for (UINT32 whichIndx = 0; whichIndx < indexBuffer.NumIndices; ++whichIndx)
            {
                UINT32 oneIndx = whichIndx;
                switch (gltfStride)
                {
                    case 2:
                    {
                        oneIndx = static_cast<UINT32>(*reinterpret_cast<const UINT16*>(dataPtr));
                        break;
                    }
                    case 4:
                    {
                        oneIndx = *reinterpret_cast<const UINT32*>(dataPtr);
                        break;
                    }
                    default:
                    {
                        LOGE("Stride %i not supported for index buffers", gltfStride);
                        ADRENO_ASSERT(false, __FILE__, __LINE__);
                        return false;
                    }
                }

                indexBuffer.Indices[whichIndx] = oneIndx;
                dataPtr += gltfStride;
                //LOGI("indexBuffer.Indices[%i]=%i", whichIndx, indexBuffer.Indices[whichIndx]);
            }
            break;
        }
        case Adreno::ATTRIB_POSITION:
        case Adreno::ATTRIB_NORMAL:
        case Adreno::ATTRIB_TANGENT:
        case Adreno::ATTRIB_TEXCOORD_0:
        case Adreno::ATTRIB_TEXCOORD_1:
        {
            int gltfStrideSupported;
            switch (gltfVertexAttributeIndex)
            {
                case Adreno::ATTRIB_TANGENT:
                {
                    gltfStrideSupported = 16;
                    break;
                }
                case Adreno::ATTRIB_POSITION:
                case Adreno::ATTRIB_NORMAL:
                {
                    gltfStrideSupported = 12;
                    break;
                }
                case Adreno::ATTRIB_TEXCOORD_0:
                case Adreno::ATTRIB_TEXCOORD_1:
                {
                    gltfStrideSupported = 8;
                    break;
                }
                default:
                {
                    ADRENO_ASSERT(false, __FILE__, __LINE__);
                }
            }
            ADRENO_ASSERT(gltfStride == gltfStrideSupported, __FILE__, __LINE__);//presently only gltfStrideRequired bytes per vertex attribute are supported

            VertexBufferEnsureSpace(&adrenoMesh, accessorData.count);
            Adreno::VertexBuffer& vertexBuffer = adrenoMesh.Vertices;

            const Adreno::VertexProperty* const vertexProperty = vertexFormat.GetProperty(gltfVertexAttributeIndex);
            const float* const gltfAttributeBuffer = reinterpret_cast<const float* const>(dataPtr);
            const size_t numFloatsPerVertexAttribute = gltfStride >> 2;
            for (UINT32 vertexAttributeIndex = 0; vertexAttributeIndex < accessorData.count; ++vertexAttributeIndex)
            {
                const size_t verticesBufferIndex = vertexProperty->Offset + vertexAttributeIndex * vertexFormat.Stride;
                float* const adrenoMeshVertexAttribute = reinterpret_cast<float* const>(&adrenoMesh.Vertices.Buffer[verticesBufferIndex]);
                //LOGI("adrenoMesh.Vertices.Buffer[%zu], adrenoMesh.Vertices.BufferSize=%u", verticesBufferIndex, adrenoMesh.Vertices.BufferSize);
                for (size_t floatIndex = 0; floatIndex < numFloatsPerVertexAttribute; ++floatIndex)
                {
                    adrenoMeshVertexAttribute[floatIndex] = gltfAttributeBuffer[numFloatsPerVertexAttribute * vertexAttributeIndex + floatIndex];
                    //LOGI("adrenoMeshVertexAttribute[%zu]=%f", floatIndex, adrenoMeshVertexAttribute[floatIndex]);
                }

                //LOGI(   "vertexProperty->Offset=%u, vertexFormat.Stride=%u, gltfVertexAttributeIndex=%zu/%s, vertexProperty->Type=%i, numFloatsPerVertexAttribute=%zu, adrenoMeshVertexAttribute[%i]=",
                //        vertexProperty->Offset, vertexFormat.Stride, gltfVertexAttributeIndex, Adreno::s_gltfAttribsString[gltfVertexAttributeIndex], 
                //        vertexProperty->Type, numFloatsPerVertexAttribute, vertexAttributeIndex);
                //LOGI("[");
                //LOGI(" %f,", adrenoMeshVertexAttribute[0]);
                //LOGI(" %f,", adrenoMeshVertexAttribute[1]);
                //if (numFloatsPerVertexAttribute == 3)
                //{
                //    LOGI(" %f", adrenoMeshVertexAttribute[2]);
                //}
                //LOGI("]");
            }
            break;
        }
        case Adreno::ATTRIB_COLOR_0:
        {
            ADRENO_ASSERT((gltfStride == 8) || (gltfStride == 16), __FILE__, __LINE__);//presently only 8 or 16 bytes-per-vertex supported

            VertexBufferEnsureSpace(&adrenoMesh, accessorData.count);
            Adreno::VertexBuffer& vertexBuffer = adrenoMesh.Vertices;

            LOGE("%s not implemented yet; nothing written", Adreno::s_gltfAttribsString[Adreno::ATTRIB_COLOR_0]);

            if (gltfStride == 8)
            {
                //// Data is UNSIGNED_SHORT
                //uint16_t R, G, B, A;
                //uint16_t* pUShortColor = (uint16_t*)AttribInfo[Adreno::ATTRIB_COLOR_0].pData;

                //R = pUShortColor[OneIndx * 4 + 0];
                //G = pUShortColor[OneIndx * 4 + 1];
                //B = pUShortColor[OneIndx * 4 + 2];
                //A = pUShortColor[OneIndx * 4 + 3];

                //if (G != 0)
                //{
                //    uint32_t uiDebug = 1999;
                //}

                //// Convert from USHORT to FLOAT
                //vertex.color[0] = (float)R / 65535.0f;
                //vertex.color[1] = (float)G / 65535.0f;
                //vertex.color[2] = (float)B / 65535.0f;
                //vertex.color[3] = (float)A / 65535.0f;

            }
            else if (gltfStride == 16)
            {
                // Data is FLOAT?
                //float* pFloatColor = (float*)AttribInfo[Adreno::ATTRIB_COLOR_0].pData;

                //vertex.color[0] = pFloatColor[OneIndx * 4 + 0];
                //vertex.color[1] = pFloatColor[OneIndx * 4 + 1];
                //vertex.color[2] = pFloatColor[OneIndx * 4 + 2];
                //vertex.color[3] = pFloatColor[OneIndx * 4 + 3];
            }
            else
            {
                LOGE("Unsupported %s element stride=%i -- no data written", Adreno::s_gltfAttribsString[Adreno::ATTRIB_COLOR_0], gltfStride);
                return false;
            }
            break;
        }
        default:
        {
            LOGI("gltfVertexAttribute %zu/%s not supported; no data written", gltfVertexAttributeIndex, gltfVertexAttributeString);
            break;
        }
    }



    return true;
}

static void GltfParseGeometryImpl(
    Adreno::VertexFormat* const vertexFormatPtr,
    Adreno::Mesh* const adrenoMeshPtr,
    const tinygltf::Model& gltfModel,
    const int accessorIndx,///<index into the gltf accessors array that contains the vertex attribute data and associated gltf metadata
    const char* const gltfVertexAttributeString,
    const Adreno::GltfAttribs gltfVertexAttributeIndex)
{
    //either parse vertex format or adreno mesh; current use cases would make trying to do both simultaneously an error
    if (vertexFormatPtr)
    {
        assert(!adrenoMeshPtr);
        GltfParseVertexProperty(vertexFormatPtr, gltfModel, accessorIndx, gltfVertexAttributeString, gltfVertexAttributeIndex);
    }
    else
    {
        assert(!vertexFormatPtr);
        GltfParseMesh(adrenoMeshPtr, gltfModel, accessorIndx, gltfVertexAttributeString, gltfVertexAttributeIndex);
    }
}
static void GltfParseGeometry(
    Adreno::VertexFormat* const vertexFormatPtr,
    Adreno::Mesh* const adrenoMeshPtr,
    const tinygltf::Model& gltfModel,
    const std::map<std::string, int>& gltfAttributes,
    const int primitiveDataIndices)///<index into the gltf accessors array that contains the index buffer and associated gltf metadata
{
    //either parse vertex format or adreno mesh; current use cases would make trying to do both simultaneously an error
    assert(reinterpret_cast<uintptr_t>(vertexFormatPtr) ^ reinterpret_cast<uintptr_t>(adrenoMeshPtr));
    assert(primitiveDataIndices >= 0);

    //indices are special case in GLTF
    const Adreno::GltfAttribs gltfVertexAttributeIndexIndices = Adreno::ATTRIB_INDICES;
    GltfParseGeometryImpl(
        vertexFormatPtr, 
        adrenoMeshPtr, 
        gltfModel, 
        primitiveDataIndices,
        Adreno::s_gltfAttribsString[gltfVertexAttributeIndexIndices],
        gltfVertexAttributeIndexIndices);

    //read vertex attributes
    for (auto attribIter = gltfAttributes.cbegin(); attribIter != gltfAttributes.cend(); attribIter++)
    {
        for (size_t gltfVertexAttributeIndex = Adreno::ATTRIB_FIRST_VALUE; gltfVertexAttributeIndex < Adreno::NUM_GLTF_ATTRIBS; ++gltfVertexAttributeIndex)
        {
            const char* const gltfVertexAttributeString = Adreno::s_gltfAttribsString[gltfVertexAttributeIndex];
            if (attribIter->first.compare(gltfVertexAttributeString) == 0)
            {
                GltfParseGeometryImpl(
                    vertexFormatPtr,
                    adrenoMeshPtr,
                    gltfModel, 
                    attribIter->second,
                    gltfVertexAttributeString, 
                    static_cast<Adreno::GltfAttribs>(gltfVertexAttributeIndex));
            }
        }
    }
}

static bool GltfValidIndex(const int i)
{
    return i >= 0;//negative value signifies invalid
}

static void GLTFLoaderSetup(tinygltf::TinyGLTF*const loaderPtr)
{
    ADRENO_REF(loaderPtr, loader);
    loader.SetImageLoader(&StubGltfLoadImageDataFunction, nullptr);
    loader.SetFsCallbacks(tinygltf::FsCallbacks{ &Gltf_FileExists, &Gltf_ExpandFilePath, &Gltf_ReadWholeFile, &Gltf_WriteWholeFile, NULL });
}
static bool GltfModelNodeMeshIndexValid(const int gltfModelNodeMeshIndex, const size_t gltfModelMeshesSize)
{
    return gltfModelNodeMeshIndex >= 0 && gltfModelNodeMeshIndex < gltfModelMeshesSize;
}
static void GltfFilenameFromImage(std::string* const textureFilenamePtr, const tinygltf::Image& image)
{
    ADRENO_REF(textureFilenamePtr, textureFilename);

    const char* const mimeTypePrefix = "image/";
    const uint32_t strlenMimeTypePrefix = strlen(mimeTypePrefix);
    //LOGI( "image.mimeType.c_str()=%s, image.name.c_str()=%s, image.uri.c_str()=%s",
    //      image.mimeType.c_str(), image.name.c_str(), image.uri.c_str());
    const bool mimeTypePresent = strncmp(image.mimeType.c_str(), mimeTypePrefix, strlenMimeTypePrefix) == 0;

    const size_t adrenoNamedIdMaxLengthMinusOne = CastWithAssert<int, size_t>(Adreno::NamedId::MAX_NAME_LENGTH - 1);
    if (mimeTypePresent)
    {
        textureFilename.reserve(image.name.length() + 4);//four characters for extension, like ".png"
        textureFilename = image.name;
        textureFilename += std::string(".");
        textureFilename += std::string(&image.mimeType.c_str()[strlenMimeTypePrefix]);
    }
    else
    {
        ADRENO_ASSERT(image.uri.length() > 0, __FILE__, __LINE__);
        textureFilename = image.uri;
    }
}
static unsigned long CStringHash(const std::string& stlString)
{
    const char*s = stlString.c_str();

    unsigned long hash = 5381;
    int c;

    while ((c = *s++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}
struct CStringHashFunctor
{
    unsigned long operator()(const std::string& s) const
    {
        return CStringHash(s);
    }
};
static int GltfTextureIndexResolve(int textureIndex, const std::vector<tinygltf::Texture>& gltfModelTextures)
{
    if (!GltfValidIndex(textureIndex))
    {
        //LOGI("gltf texture index=%i -- adreno material not created, since presently only this texture is supported", textureIndex);
        return -1;
    }
    else
    {
        return gltfModelTextures[textureIndex].source;
    }
}
static bool GLTFModelsLoad(
    std::vector <Adreno::Model>* const adrenoModelsPtr,
    std::vector <std::string>* const materialFilepathsPtr,
    Adreno::MeshesSortedByMaterialId*const meshesSortedByMaterialIdPtr,///<optional
    const CHAR* const filename, 
    const tinygltf::Model& gltfModel,
    const std::string& err, 
    const std::string& warn,
    bool gltfModelValid,
    const UINT32 baseColorDefault_MaterialId)
{
    ADRENO_REF(adrenoModelsPtr, adrenoModels);
    ADRENO_REF(materialFilepathsPtr, materialFilePaths);

    if (!warn.empty())
    {
        LOGW("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) 
    {
        LOGE("Err: %s\n", err.c_str());
    }

    if (!gltfModelValid)
    {
        LOGE("Failed to parse glTF\n");
        return false;
    }

    // Parse the contents of the gltfModel into Adreno::Model
    gltfAttribInfo attribInfo[Adreno::NUM_GLTF_ATTRIBS];

    const tinygltf::Scene& sceneData = gltfModel.scenes[gltfModel.defaultScene];

    //determine number of Adreno models
    size_t adrenoModelsNum = 0;
    const size_t gltfModelNodesSize = gltfModel.nodes.size();
    ADRENO_ASSERT(gltfModelNodesSize > 0, __FILE__, __LINE__);

    const size_t gltfModelMeshesSize = gltfModel.meshes.size();
    ADRENO_ASSERT(gltfModelMeshesSize > 0, __FILE__, __LINE__);

    for (size_t gltfModelNodeIndex = 0; gltfModelNodeIndex < gltfModelNodesSize; ++gltfModelNodeIndex)
    {
        if (GltfModelNodeMeshIndexValid(gltfModel.nodes[gltfModelNodeIndex].mesh, gltfModelMeshesSize))
        {
            ++adrenoModelsNum;
        }
    }
    //LOGI("adrenoModelsNum=%zu", adrenoModelsNum);
    ADRENO_ASSERT(adrenoModelsNum > 0, __FILE__, __LINE__);
    adrenoModels.resize(adrenoModelsNum);
    size_t gltfModelNodeIndex = 0;
    size_t adrenoModelIndex = 0;
    const tinygltf::Node* gltfNodeDataPtr = nullptr;
    std::unordered_map<std::string, UINT32, CStringHashFunctor> textureFilenameContainer;
    while(true)//traverse gltf nodes, mapping each gltf mesh to adreno model (since gltf mesh contains n meshes, and adreno model's contain n meshes)
    {
        //find the next valid gltf node, if it exists
        bool gltfNodeDataPtrValid = false;
        while (!gltfNodeDataPtrValid && gltfModelNodeIndex < gltfModelNodesSize)
        {
            gltfNodeDataPtr = &gltfModel.nodes[gltfModelNodeIndex++];
            gltfNodeDataPtrValid = GltfModelNodeMeshIndexValid(gltfNodeDataPtr->mesh, gltfModelMeshesSize);
        }

        if (!gltfNodeDataPtrValid)
        {
            const bool processedExpectedNumberOfAdrenoModelsAndAllGLTFNodes = adrenoModelIndex == adrenoModelsNum && gltfModelNodeIndex >= gltfModelNodesSize;
            ADRENO_ASSERT(processedExpectedNumberOfAdrenoModelsAndAllGLTFNodes, __FILE__, __LINE__);
            return processedExpectedNumberOfAdrenoModelsAndAllGLTFNodes;
        }
        else//gltfNodeDataPtrValid
        {
            if (gltfNodeDataPtr->mesh >= gltfModel.meshes.size())
            {
                LOGE("Error loading %s: SkeletonNodeData mesh=%i is invalid index, because gltfModel.meshes.size()=%zu.  gltfModel.scenes.size()=%zu, gltfModel.nodes.size()=%zu, gltfModel.defaultScene=%i, sceneData.nodes[0]=%i",
                    filename, gltfNodeDataPtr->mesh, gltfModel.meshes.size(), gltfModel.scenes.size(), gltfModel.nodes.size(), gltfModel.defaultScene, sceneData.nodes[0]);
                ADRENO_ASSERT(false, __FILE__, __LINE__);
                return false;
            }

            const tinygltf::Mesh& gltfMeshData = gltfModel.meshes[gltfNodeDataPtr->mesh];
            const size_t gltfPrimitivesSize = gltfMeshData.primitives.size();
            
            //add any new texture maps to global pool
            for (size_t primitivesIndex = 0; primitivesIndex < gltfPrimitivesSize; ++primitivesIndex)
            {
                const int gltfMaterialId = gltfMeshData.primitives[primitivesIndex].material;
                if (GltfValidIndex(gltfMaterialId))
                {
                    const int baseColorTextureIndex =
                        GltfTextureIndexResolve(gltfModel.materials[gltfMaterialId].pbrMetallicRoughness.baseColorTexture.index, gltfModel.textures);
                    const int imageIndex = gltfModel.textures[baseColorTextureIndex].source;
                    ADRENO_ASSERT(GltfValidIndex(imageIndex), __FILE__, __LINE__);
                    std::string textureFilename;
                    GltfFilenameFromImage(&textureFilename, gltfModel.images[imageIndex]);

                    if (textureFilenameContainer.find(textureFilename) == textureFilenameContainer.end())
                    {
                        textureFilenameContainer[textureFilename] = materialFilePaths.size();
                        materialFilePaths.push_back(textureFilename);
                    }
                }
            }
            //LOGI("%zu more primitive(s) increased the total number of material references to %zu", gltfPrimitivesSize, textureFilenameContainer.size());

            auto& adrenoModel = adrenoModels[adrenoModelIndex++];
            adrenoModel.ResizeMeshes(gltfPrimitivesSize);
            for (size_t meshIndex = 0; meshIndex < adrenoModel.NumMeshes; ++meshIndex)
            {
                const tinygltf::Primitive& primitiveData = gltfMeshData.primitives[meshIndex];
                Adreno::Mesh& adrenoMesh = *adrenoModel.MeshGet(meshIndex);
                Adreno::VertexFormat& vertexFormat = adrenoMesh.Vertices.Format;
                vertexFormat.ResizeProperties(Adreno::NUM_GLTF_ATTRIBS);

                //indices are special case in GLTF; parse them first while filling out the vertex attribute format
                GltfParseGeometry(&vertexFormat, nullptr, gltfModel, primitiveData.attributes, primitiveData.indices);

                //with the vertex attribute format defined, we can read in the rest of the vertex attributes in a buffer that interleaves said vertex attributes
                GltfParseGeometry(nullptr, &adrenoMesh, gltfModel, primitiveData.attributes, primitiveData.indices);

                UINT32 adrenoMeshSurfaceMaterialId = baseColorDefault_MaterialId;
                std::string textureFilename("baseColorDefault");
                const int gltfMaterialId = primitiveData.material;
                //LOGI("primitiveData.material=%i", gltfMaterialId);
                if (GltfValidIndex(gltfMaterialId))
                {
                    const auto& gltfMaterial = gltfModel.materials[gltfMaterialId];
                    const int baseColorTextureIndex = 
                        GltfTextureIndexResolve(gltfMaterial.pbrMetallicRoughness.baseColorTexture.index, gltfModel.textures);
                    if (GltfValidIndex(baseColorTextureIndex))
                    {
                        const int imageIndex = gltfModel.textures[baseColorTextureIndex].source;
                        ADRENO_ASSERT(GltfValidIndex(imageIndex), __FILE__, __LINE__);
                        
                        GltfFilenameFromImage(&textureFilename, gltfModel.images[imageIndex]);
                        adrenoMeshSurfaceMaterialId = textureFilenameContainer[textureFilename];
                    }
                }
                adrenoMesh.Surfaces.Resize(1);//presently support just one surface per mesh
                Adreno::MeshSurface& adrenoMeshSurface = adrenoMesh.Surfaces.Surfaces[0];
                adrenoMeshSurface.MaterialId = adrenoMeshSurfaceMaterialId;//purposefully don't fill out Adreno::Model::Materials -- MaterialId here is the index into the global texture handle array
                adrenoMeshSurface.StartIndex = 0;
                adrenoMeshSurface.NumTriangles = 0;//presently apply the texture to all triangles in this mesh

                if (meshesSortedByMaterialIdPtr)
                {
                    UINT32 vertexFormatBitfield = 0;
                    for(INT32 vertexFormatPropertyIndex = 0; vertexFormatPropertyIndex < vertexFormat.NumProperties; ++vertexFormatPropertyIndex)
                    {
                        for(size_t gltfAttrib = Adreno::GltfAttribs::ATTRIB_FIRST_VALUE; 
                            gltfAttrib < Adreno::GltfAttribs::NUM_GLTF_ATTRIBS; 
                            ++gltfAttrib)
                        {
                            const Adreno::VertexProperty& vertexProperty = *vertexFormat.GetProperty(vertexFormatPropertyIndex);
                            if (vertexProperty.Valid())
                            {
                                if(Adreno::VertexPropertyIs(vertexProperty.Usage.Name, static_cast<Adreno::GltfAttribs>(gltfAttrib)))
                                {
                                    const UINT32 bit = 1 << gltfAttrib;
                                    ADRENO_ASSERT((vertexFormatBitfield & bit) == 0, __FILE__, __LINE__);//vertex attributes should not repeat
                                    vertexFormatBitfield |= bit;
                                }
                            }
                        }
                    }
                    //LOGI("vertexFormatBitfield=%x", vertexFormatBitfield);
                    (*meshesSortedByMaterialIdPtr)[adrenoMeshSurface.MaterialId][vertexFormatBitfield].push_back(&adrenoMesh);
                }

                adrenoModel.NumJoints = 0;
                adrenoModel.Joints = nullptr;
                
#if 0//print out all supported elements of loaded AdrenoModel
                {
                    const char* const dividerStr = "---------";
                    LOGI("%sGLTF Model %zu loaded from %s:", dividerStr, adrenoModelIndex, filename);

                    LOGI("adrenoModel.NumMeshes=%i", adrenoModel.NumMeshes);
                    //sleep(1);//give logcat plenty of time (1000ms!) to drain before issuing many logcat calls (sad that this must be done, but often it must, or logcat randomly fails to output text)
                    for (size_t adrenoMeshIndex = 0; adrenoMeshIndex < adrenoModel.NumMeshes; ++adrenoMeshIndex)
                    {
                        const Adreno::Mesh& adrenoMesh = adrenoModel.Meshes[adrenoMeshIndex];

                        LOGI("%sGLTF Mesh %zu from GLTF Model %zu from %s:", dividerStr, adrenoMeshIndex, adrenoModelIndex, filename);
                        const Adreno::MeshSurfaceArray& adrenoMeshSurfaceArray = adrenoMesh.Surfaces;
                        LOGI("adrenoMeshSurfaceArray.NumSurfaces=%u", adrenoMeshSurfaceArray.NumSurfaces);
                        for (size_t i = 0; i < adrenoMeshSurfaceArray.NumSurfaces; ++i)
                        {
                            const Adreno::MeshSurface& adrenoMeshSurface = adrenoMeshSurfaceArray.Surfaces[i];
                            const Adreno::Material& adrenoMaterial = *adrenoModel.MaterialGet(adrenoMeshSurface.MaterialId);
                            LOGI("adrenoMeshSurface.StartIndex=%u, adrenoMeshSurface.NumTriangles=%u, adrenoMeshSurface.MaterialId=%u=[%s,%i]",
                                adrenoMeshSurface.StartIndex, adrenoMeshSurface.NumTriangles, adrenoMeshSurface.MaterialId, adrenoMaterial.Id.Name,
                                adrenoMaterial.Id.Id);
                        }

                        const Adreno::IndexBuffer& indexBuffer = adrenoMesh.Indices;
                        LOGI("Indices.NumIndices=%u", indexBuffer.NumIndices);
#if 0
                        for (size_t i = 0; i < indexBuffer.NumIndices; ++i)
                        {
                            LOGI("\tadrenoMesh.Indices[%zu]=%u", i, indexBuffer.Index(i));
                        }
#endif

                        const Adreno::VertexBuffer& vertexBuffer = adrenoMesh.Vertices;
                        LOGI("%sVertices.NumVerts=%u, Buffer=%p,BufferSize=%u", dividerStr, vertexBuffer.NumVerts, vertexBuffer.Buffer, vertexBuffer.BufferSize);

                        const Adreno::VertexFormat& vertexFormat = vertexBuffer.Format;
                        LOGI("%sVertices.Format:NumProperties=%i,Stride=%i", dividerStr, vertexFormat.NumProperties, vertexFormat.Stride);
                        const size_t vertexFormatIndexStart = 1;//skip indices; they're a special case
                        for (size_t i = vertexFormatIndexStart; i < vertexFormat.NumProperties; ++i)
                        {
                            const Adreno::VertexProperty& vertexProperty = *vertexFormat.GetProperty(i);
                            if (CStringNotEmpty(&vertexProperty.Usage.Name[0]))
                            {
                                LOGI("\t%zu/%s:VertexPropertyType=%i,Offset=%u,Size=%u,NumValues=%u,IsNormalized=%i",
                                    i, &vertexProperty.Usage.Name[0], vertexProperty.Type, vertexProperty.Offset, vertexProperty.Size(), vertexProperty.NumValues(), vertexProperty.IsNormalized());
                            }
                        }

                        LOGI("%sVertex Attribute Information Dump:", dividerStr);
                        for (size_t vertexFormatIndex = vertexFormatIndexStart; vertexFormatIndex < vertexFormat.NumProperties; ++vertexFormatIndex)
                        {
                            const Adreno::VertexProperty& vertexProperty = *vertexFormat.GetProperty(vertexFormatIndex);
                            const INT32 vertexPropertyNumValues = vertexProperty.NumValues();
                            const char* const vertexPropertyUsageName = &vertexProperty.Usage.Name[0];
                            if (CStringNotEmpty(vertexPropertyUsageName))
                            {
                                LOGI("\t%zu/%s:VertexPropertyType=%i,Offset=%u,Size=%u,NumValues=%u,IsNormalized=%i",
                                    vertexFormatIndex, vertexPropertyUsageName, vertexProperty.Type, vertexProperty.Offset, vertexProperty.Size(), vertexPropertyNumValues, vertexProperty.IsNormalized());
                            }

                            if (vertexProperty.Size())
                            {
                                for (size_t vertexIndex = 0; vertexIndex < vertexBuffer.NumVerts; ++vertexIndex)
                                {
                                    const UINT32 vertexPropertySize = vertexProperty.Size();
                                    const float* const floats =
                                        reinterpret_cast<const float* const>(adrenoMesh.Vertices.Buffer + vertexProperty.Offset + vertexFormat.Stride * vertexIndex);
                                    switch (vertexPropertySize)
                                    {
                                        case 8:
                                        {
                                            //LOGI("\t\t%zu) [%f,%f] -- %u bytes; assume %zu floats",
                                            //    vertexIndex, floats[0], floats[1], vertexPropertySize, vertexPropertySize / sizeof(float));
                                            break;
                                        }
                                        case 12:
                                        {
                                            //LOGI("\t\t%zu) [%f,%f,%f] -- %u bytes; assume %zu floats",
                                            //    vertexIndex, floats[0], floats[1], floats[2], vertexPropertySize, vertexPropertySize / sizeof(float));
                                            break;
                                        }
                                        case 16:
                                        {
                                            //LOGI("\t\t%zu) [%f,%f,%f,%f] -- %u bytes; assume %zu floats",
                                            //    vertexIndex, floats[0], floats[1], floats[2], floats[3], vertexPropertySize, vertexPropertySize / sizeof(float));
                                            break;
                                        }
                                        default:
                                        {
                                            LOGI("\t\tUnsupported VertexPropertyType=%i of size %i bytes", vertexProperty.Type, vertexPropertySize);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
#endif

                adrenoMesh.AssertValid();
            }
        }
    }
}
bool Adreno::FrmLoadGLTFModelsFromTextFile(
    std::vector<Adreno::Model>* const adrenoModelsPtr, 
    std::vector<std::string>* const materialFilepathsPtr, 
    MeshesSortedByMaterialId* const meshesSortedByMaterialIdPtr,///<optional
    const CHAR* const filename,
    const UINT32 baseColorDefault_MaterialId)
{
    ADRENO_REF(adrenoModelsPtr, adrenoModels);
    ADRENO_REF(materialFilepathsPtr, materialFilepaths);
    ADRENO_ASSERT(CStringNotEmpty(filename), __FILE__, __LINE__);

    tinygltf::TinyGLTF loader;
    GLTFLoaderSetup(&loader);

    tinygltf::Model gltfModel;
    std::string err;
    std::string warn;

    const bool loaderRet = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);
    return GLTFModelsLoad(&adrenoModels, &materialFilepaths, meshesSortedByMaterialIdPtr, filename, gltfModel, err, warn, loaderRet, baseColorDefault_MaterialId);
}
bool Adreno::FrmLoadGLTFModelsFromBinaryFile(
    std::vector<Adreno::Model>* const adrenoModelsPtr,
    std::vector <std::string>* const materialFilepathsPtr,
    MeshesSortedByMaterialId* const meshesSortedByMaterialIdPtr,///<optional
    const CHAR* const filename,
    const UINT32 baseColorDefault_MaterialId)
{
    ADRENO_REF(adrenoModelsPtr, adrenoModels);
    ADRENO_REF(materialFilepathsPtr, materialFilepaths);
    ADRENO_ASSERT(CStringNotEmpty(filename), __FILE__, __LINE__);

    tinygltf::TinyGLTF loader;
    GLTFLoaderSetup(&loader);

    tinygltf::Model gltfModel;
    std::string err;
    std::string warn;

    const bool loaderRet = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
    return GLTFModelsLoad(&adrenoModels, &materialFilepaths, meshesSortedByMaterialIdPtr, filename, gltfModel, err, warn, loaderRet, baseColorDefault_MaterialId);
}
//-----------------------------------------------------------------------------
static void Memcpy(
    INT32* const bytesIndexPtr, 
    UINT8*const destinationBuffer, 
    const UINT8*const sourceBuffer, 
    const UINT32 elementsNum, 
    const UINT32 stridePerElement)
{
    ADRENO_REF(bytesIndexPtr, bytesIndex);
    ADRENO_ASSERT(destinationBuffer, __FILE__, __LINE__);
    ADRENO_ASSERT(sourceBuffer, __FILE__, __LINE__);
    ADRENO_ASSERT(elementsNum > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(stridePerElement > 0, __FILE__, __LINE__);

    const INT32 bytesNum = CastWithAssert<UINT32, INT32>(stridePerElement * elementsNum);
    memcpy(&destinationBuffer[bytesIndex], sourceBuffer, bytesNum);
    bytesIndex += bytesNum;
}
void Adreno::FrmMeshesMergeIfSameMaterial(
    Adreno::Model*const modelMergedPtr, 
    const MeshesSortedByMaterialId& meshesSortedByMaterialId)
{
    ADRENO_REF(modelMergedPtr, modelMerged);
    
    size_t meshesSortedByMaterialIdAndVertexPropertySize = 0;
    for (const auto& meshesSortedByMaterialIdPair : meshesSortedByMaterialId)
    {
        meshesSortedByMaterialIdAndVertexPropertySize += meshesSortedByMaterialIdPair.second.size();
    }
    ADRENO_ASSERT(meshesSortedByMaterialIdAndVertexPropertySize > 0, __FILE__, __LINE__);

    size_t modelMergedMeshIndex = 0;
    modelMerged.ResizeMeshes(CastWithAssert<size_t,int>(meshesSortedByMaterialIdAndVertexPropertySize));
    //LOGI("meshesSortedByMaterialId.size()=%zu", meshesSortedByMaterialId.size());
    for (const auto& meshesSortedByMaterialIdPair : meshesSortedByMaterialId)
    {
        const MeshesSortedByVertexFormat& meshesSortedByVertexFormat = meshesSortedByMaterialIdPair.second;
        for (const auto& meshesSortedByVertexFormatPair : meshesSortedByVertexFormat)
        {        
            const MeshPtrContainer& meshPtrs = meshesSortedByVertexFormatPair.second;
            ADRENO_ASSERT(meshPtrs.size() > 0, __FILE__, __LINE__);

            //LOGI("meshPtrs.size()=%zu", meshPtrs.size());

            //count vertices and indices
            UINT32 verticesNum = 0;
            UINT32 indicesNum = 0;
            const auto& meshZero = *meshPtrs[0];
            const auto& meshZeroVertexFormat = meshZero.Vertices.Format;
            for (const Adreno::Mesh* const meshPtr : meshPtrs)
            {
    #if ADRENO_DEBUG
                //verify all meshes in this container share the right traits to be batched into one draw call
                ADRENO_ASSERT(meshPtr->Surfaces.NumSurfaces == 1, __FILE__, __LINE__);
                ADRENO_ASSERT(meshZero.Surfaces.SurfaceGet(0)->MaterialId == meshPtr->Surfaces.SurfaceGet(0)->MaterialId, __FILE__, __LINE__);

                const auto& meshPtrVertexFormat = meshPtr->Vertices.Format;
                ADRENO_ASSERT(meshZeroVertexFormat.Stride == meshPtrVertexFormat.Stride, __FILE__, __LINE__);
                ADRENO_ASSERT(meshZeroVertexFormat.NumProperties == meshPtrVertexFormat.NumProperties, __FILE__, __LINE__);

                for(size_t i=0; i < meshPtrVertexFormat.NumProperties; ++i)
                {                    
                    ADRENO_ASSERT(meshZeroVertexFormat.Properties[i] == meshPtrVertexFormat.Properties[i], __FILE__, __LINE__);
                }            
    #endif//#if ADRENO_DEBUG

                verticesNum += meshPtr->Vertices.NumVerts;
                indicesNum += meshPtr->Indices.NumIndices;
            }

            //LOGI("verticesNum=%u, indicesNum=%u", verticesNum, indicesNum);

            //merge meshes
            auto& meshMerged = *modelMerged.MeshGet(modelMergedMeshIndex++);
            meshMerged.Vertices.Format = meshZeroVertexFormat;
            
            meshMerged.Surfaces.Resize(1);
            *meshMerged.Surfaces.SurfaceGet(0) = *meshZero.Surfaces.SurfaceGet(0);//shallow copy sufficient

            //copy index and vertex buffers
            meshMerged.Vertices.InitializeBuffer(verticesNum);
            meshMerged.Indices.Resize(indicesNum);
            UINT32 verticesWrittenNum = 0;
            UINT32 indicesWrittenNum = 0;
            INT32 vertexBytesIndex = 0;
            INT32 indexBytesIndex = 0;
            for (const Adreno::Mesh* const meshPtr : meshPtrs)
            {
                const UINT32 meshPtrNumVerts = meshPtr->Vertices.NumVerts;
                Memcpy(
                    &vertexBytesIndex, 
                    meshMerged.Vertices.Buffer, 
                    meshPtr->Vertices.Buffer, 
                    meshMerged.Vertices.Format.Stride, 
                    meshPtrNumVerts);
                
                const UINT32 meshPtrNumIndices = meshPtr->Indices.NumIndices;
                Memcpy(
                    &indexBytesIndex,
                    reinterpret_cast<UINT8*>(meshMerged.Indices.Indices), 
                    reinterpret_cast<const UINT8*>(meshPtr->Indices.Indices),
                    CastWithAssert<size_t, UINT32>(sizeof(meshMerged.Indices.Indices[0])), 
                    meshPtrNumIndices);

                const UINT32 indexBufferIndexEnd = indicesWrittenNum + meshPtrNumIndices;
                for (UINT32 indexBufferIndex = indicesWrittenNum; indexBufferIndex < indexBufferIndexEnd; ++indexBufferIndex)
                {
                    meshMerged.Indices.Indices[indexBufferIndex] += verticesWrittenNum;
                }
                verticesWrittenNum += meshPtrNumVerts;
                indicesWrittenNum += meshPtrNumIndices;
            }
        }
    }
}
//-----------------------------------------------------------------------------
void Adreno::FrmKtxFilesLoad(ResourceHandleArray<1024>*const textureHandlesPtr, const std::vector<std::string>& textureFilepaths)
{
    ADRENO_REF(textureHandlesPtr, textureHandles);
    ADRENO_ASSERT(g_pAssetManager, __FILE__, __LINE__);
    
    GLuint textureHandleIndex = 0;
    for (const auto& filenamePath : textureFilepaths)
    {
        //load texture
        //png textures (which are artist-friendly) should have been cooked into ktx format for efficient loading
        std::string filenamePathFixed(filenamePath);
        const size_t filenamePathSize = filenamePath.size();
        const char*const dotPng = ".png";
        const char*const dotKtx = ".ktx";
        const size_t dotPngStrlen = strlen(dotPng);
        ADRENO_ASSERT(dotPngStrlen == strlen(dotKtx), __FILE__, __LINE__);
        const size_t filenameExtensionIndex = filenamePathSize - dotPngStrlen;
        if( filenamePathSize >= dotPngStrlen && 
            strncmp(&filenamePath[filenameExtensionIndex], dotPng, dotPngStrlen) == 0)
        {
            for(size_t i=0; i < dotPngStrlen; ++i)
            {
                filenamePathFixed[filenameExtensionIndex + i] = dotKtx[i];
            }
        }
        //LOGI("filenamePathFixed=%s", filenamePathFixed.c_str());
        
        ADRENO_ASSERT(g_pAssetManager, __FILE__, __LINE__);
        AAsset* pAAsset = AAssetManager_open(g_pAssetManager, filenamePathFixed.c_str(), AASSET_MODE_STREAMING);
        ADRENO_ASSERT(pAAsset != nullptr, __FILE__, __LINE__);
        
        const size_t fileSizeBytes = AAsset_getLength(pAAsset);
        ADRENO_ASSERT(fileSizeBytes > 0, __FILE__, __LINE__);
        std::vector<uint8_t> bytesContainer;
        bytesContainer.resize(fileSizeBytes);
        const int bytesRead = AAsset_read(pAAsset, bytesContainer.data(), fileSizeBytes);
        ADRENO_ASSERT(bytesRead == fileSizeBytes, __FILE__, __LINE__);
        
        const ktx_uint8_t ktxIdentifier[] = KTX_IDENTIFIER_REF;
        ADRENO_ASSERT(memcmp(bytesContainer.data(), ktxIdentifier, sizeof(ktxIdentifier)) == 0, __FILE__, __LINE__);
        
        ktxTexture* texture;
        ktx_size_t offset;
        ktx_uint8_t* imageData;      
        KTX_error_code ktxResult = ktxTexture_CreateFromMemory(bytesContainer.data(), fileSizeBytes, KTX_TEXTURE_CREATE_NO_FLAGS, &texture);
        ADRENO_ASSERT(ktxResult == KTX_SUCCESS, __FILE__, __LINE__);
        ADRENO_ASSERT(!texture->isArray, __FILE__, __LINE__);

        //create texture
        GLuint& textureHandle = textureHandles[textureHandleIndex++];
        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum target, glerror;
        ktxResult = ktxTexture_GLUpload(texture, &textureHandle, &target, &glerror);
        //LOGI("ktxTexture_GLUpload() -- ktxResult=%u, textureHandle=%u, target=%u, glerror=%u", ktxResult, textureHandle, target, glerror);
        ADRENO_ASSERT(ktxResult == KTX_SUCCESS, __FILE__, __LINE__);

        glGenerateMipmap(GL_TEXTURE_2D);
        ktxTexture_Destroy(texture);
    }
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
