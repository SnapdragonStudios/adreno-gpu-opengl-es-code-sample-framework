# OpenGLES Framework Overview

## Framework

Any application running on OpenGL ES 2.0 / 3.0 hardware, such as the ones produced by this Framework, have a lot of work to do besides just issuing simple draw calls. Platform-independent code is needed to create the application, as well as to abstract file I/O, memory allocations, and the standard library. Meshes, textures, and other resources need to be loaded. Support for input, fonts, and timer code is also useful. This Framework provides the platform-abstraction and a library of common features that ease the creation of new applications.

This Framework allows for higher-level constructs that are useful to graphics applications. For example, OpenGL ES provides the API for draw calls, but has no native support for an art pipeline involving artist-created meshes. The Framework provides basic support for meshes, as well as packed resources and fonts. 

## Framework Background

The original Framework from the [Adreno GPU SDK](https://developer.qualcomm.com/software/adreno-gpu-sdk/tools) supported multiple platforms via emulation of the OpenGL ES runtime. For the purposes of this repository, all non-Android platforms have been removed and only platforms that support OpenGL ES natively will be included in the future. For this reason, the file structure of the Framework retains the hierarchy of cleanly separating the platform-specific and platform-independent code. For example, all Android platform specific code is located within the [src/Android](src/Android) folder with files tagged with an _Platform suffix.

# Features Overview

The following section will provide a brief overview of the major features and components of the Framework.

## Application Creation

Application creation is taken care of automatically by the Framework. Every supported platform has its own main entry point located in the FrmApplication_Platform.cpp file for the given platform. As an example, below is the Android main entry point: 

```

void android_main(android_app* pApp)
{
	CFrmAppContainer appContainer;
	appContainer.SetAndroidApp(pApp);
	pApp->userData = &appContainer;
	pApp->onAppCmd = CommandCallback;
	pApp->onInputEvent = InputCallback;

	g_pAssetManager = pApp->activity->assetManager;
	
	appContainer.Run();
}

```

The platform-specific main entry point instantiates and then runs a CFrmAppContainer object. This object contains a CFrmApplication object, which is a virtual class derived and implemented by the application’s main class. The CFrmAppContainer is an internal, platform-specific object that is responsible for window initialization, configuring the frame buffer to obtain a render context, and running the event handling loop. The CFrmApplication object is derived by the application, allowing the application to initialize itself, render a scene, and clean everything up. 

As demonstrated in the [Hello Framework](samples/hello_fw) sample, this class is used and derived via a CSample class. The derived class is responsible for construction, initialization, handling resize events, updating and rendering the scene, and cleanup. 

```

class CSample : public CFrmApplication
{
    ...

 public:

    CSample( const CHAR* strName );

     virtual BOOL Initialize();
     virtual BOOL Resize();
     virtual VOID Update();
     virtual VOID Render();
     virtual VOID Destroy();
};

 ```


The CSample object is instantiated and created in a global function called FrmCreateApplicationInstance(). **All samples using the Framework must supply this function**, or else the sample will fail to build. The Framework uses this function as an abstracted way to create the sample, since it otherwise has no knowledge about what the CSample class is, nor how to construct it. This design was also chosen as a way to ease platform independence, since not all platforms create applications objects in the same way nor in the same order during startup. 

```

CFrmApplication* FrmCreateApplicationInstance()
{
    return new CSample( "Sample Name" );
}

 ```


The CSample constructor is, as one would expect, the appropriate place to initialize member variables. Additionally, it can be used to override inherited members from the base CFrmApplication class, like the display properties. In the future, more display attributes, like frame buffer and depth-stencil bit depths, will be exposed. 

NOTE: The member variable m_OpenGLESVersion defaults to GLES2. 
However, you can set it to GLES3 in order to enable OpenGLES 3.0 functionality. 

## Framebuffer Configuration

The frame buffer is configured for various display modes through the CFrmRenderContext::Create() method. The configuration parameters are set based on the incoming display modes and the window surface is created through EGL calls. 

## File I/O Abstraction

All Operating System calls such as file I/O are wrapped and abstracted to achieve platform independence. For instance, FrmFile.h contains a method called FrmFile_Open() which abstracts the opening of files. A developer might be tempted to call fopen() or CreateFile() instead, but the benefit of using FrmFile_Open() is that it is guaranteed to function on all operating systems that are supported by the Framework. 

## C Standard Library Abstraction

All functions in stdlib.h are wrapped to achieve platform independence. These include memory management functions such as FrmMalloc, and FrmFree, as well as cstring wrapper functions such as FrmStrcpy and FrmSprintf. 

## Input Handling

The Framework provides a simple, abstracted feature to trap and respond to key presses on the target hardware. All keys that are currently being held down, or were pressed since the last frame, can be queried by calling FrmGetInput(). 
As an example: 

```

VOID CSample::Update()
{
    UINT32 nButtons;
    UINT32 nPressedButtons;

    FrmGetInput( &m_Input, &nButtons, &nPressedButtons );
    if( nButtons & INPUT_KEY_0 )
    {
        // Do something if the '0' key is pressed
    }
}
 
```

## 2D/3D Math Support

FrmMath.h contains a variety of helper math structures and functions including a number of useful vector and matrix operations. The most commonly used types will be FRMVECTOR3 and FRMVECTOR4, for vectors and quaternions, and FRMMATRIX4X4 for GLSL-compatible matrix transforms. 

## Packed Resources

Loading textures during runtime naively requires a large amount to code to process numerous image file formats, such as TGA, BMP, and JPG. The code is not only large, but slow as well. To further complicate matters, any performance-minded application will want to use compressed textures, although compressing textures on a mobile device can take many seconds per texture. 

A better design is to process image files offline, compressing and storing them in a pre-digested binary file that enables fast load times and minimal runtime code support. Additionally, there is desire to lump several such pre-digested images together in a single file, to further facilitate fast loading and easy management of a large number of textures. 

To accommodate this design, the [Adreno GPU SDK](https://developer.qualcomm.com/software/adreno-gpu-sdk/tools) includes a tool called the ResourcePacker. The application submits an XML description of what files it wants the ResoourcePacker to process and how it should process this. Typically, this is a list of texture source files (TGA, BMP, JPG, etc.) to convert into an OpenGL ES 2.0 binary texture format (such as ATC, the AMD Texture Compression format). The output of the ResourcePacker is a single .pak file. This pack file can be accessed during runtime via the CFrmPackedResource class. Each resource is associated with a user-supplied ASCII name that can be extracted into a wrapped resource class like CFrmTexture. 

```

CFrmPackedResource resource;
if( FALSE == resource.LoadFromFile( "Media\\Textures.pak" ) )
    return FALSE;
....
CFrmTexture* pLogoTexture = resource.GetTexture( "Logo" );
}
 
 ```

A primary benefit of using packed resource files is the ability to abstract texture names from actual files. For example, artist-created meshes often embed specific textures file names that can create conflicts. For example, if TROLL.MESH and OGRE.MESH both refer to a texture called “face.bmp”, the resource packer can help avoid name conflicts since troll textures will be stored in troll.pak and and ogre textures will be stored in ogre.pak. 

## Text Rendering

The [Adreno GPU SDK](https://developer.qualcomm.com/software/adreno-gpu-sdk/tools) also includes a tool called the FontMaker which is used to rasterize TrueType fonts into a bitmap and data file that the Framework font class can use to render text. The output from the FontMaker tool is three source files used as input to create a single, binary packed resource file used during runtime. The three source files are: a TGA image file, a binary font data file and an xml file. The TGA file is a 32-bit RGBA image of the rasterized font glyphs and can be post-edited in an image editor to add coloring and custom glyphs. At run-time, the TGA image will become a 2D texture. The binary font file contains texture coordinates as well as spacing information for each glyph in the font. The xml file is used as input to the ResourcePacker, which is invoked while building the sample to build a resulting .pak file. 
During the sample’s initialization, the .pak file is used to the load the font into a CFrmFont helper class. From then on, text can be rendered in screen-space coordinates using the Draw Text function, as shown here: 

```

CFrmFont Font; 
BOOL bResult = Font.Create( "Media\\Fonts\\SomeFont16.pak" );

...

Font.DrawText( 10, 10, FRMCOLOR_WHITE, "Draw this text." );

 ```

## GLSL Shader Support

The Framework provides assistance to load, compile, and link GLSL shader programs into the application. All these can be done through a single interface FrmCompileShaderProgramFromFile(). This helper function simplifies about 100 lines of code down to a single function call.

Additionally, the helper function allows the caller to supply a list FRM_SHADER_ATTRIBUTE structures that assists the binding of vertex attributes between the application and the shader program. 

After compilation, normal GL function calls can be used to extract uniform locations, bind the shader for rendering, and so on. An example of using shaders in the Framework is shown here: 

```

FRM_SHADER_ATTRIBUTE pShaderAttributes[] =
{ 
    { "In_Position",    FRM_VERTEX_POSITION },
    { "In_Normal",      FRM_VERTEX_NORMAL },
    { "In_TexCoord",    FRM_VERTEX_TEXCOORD0 },
};

GLuint hShader;
if( FALSE == FrmCompileShaderProgramFromFile(
                              "Media\\Shaders\\Shader.vs",                                                              
                              "Media\\Shaders\\Shader.fs",
                              &hShader,
                              pShaderAttributes, 3 ) )
    return FALSE;

// Make a record of the location for each shader constant
slotWorldMat = glGetUniformLocation( hShader, "matWorld" );
....

 ```

## Geometry Meshes

The Framework provides runtime data structures for storing geometry meshes, which even includes animation data. The data structures are loaded verbatim from a binary mesh file, which was designed for run-time performance and minimal loading effort. Specifically, mesh files are comprised of two sections: system memory data structures that need to remain memory resident for the life of the mesh, and buffer initialization data that can be discarded after loading. 

Loading a mesh file requires just two memory allocations and two main file reads. Afterwards, some "pointer fixup" is required, which involves a simple traversal of the mesh data structures. Then, in a separate step, the buffer initialization data is used to create and populate vertex buffer and index buffer objects. Finally, a packed resource file can be supplied to the mesh loading code in order to bind actual textures to any references in the mesh. 

```

CFrmMesh Mesh;
if( FALSE == Mesh.Load( "Media\\Meshes\\Test.mesh" ) )
    return FALSE;

if( FALSE == Mesh.MakeDrawable( &resource ) )
     return FALSE;

``` 

Internally, a CFrmMesh object is a hierarchy of transformation frames called FRM_MESH_FRAME. Each frame may optionally include actual mesh data and pointers to child and/or sibling frames. Actual mesh rendering data is stored in FRM_MESH and FRM_MESH_SUBSET structures. The former is essentially a wrapper for a vertex buffer object, while the latter is other rendering data like materials and textures for a subset of the mesh. 

Note that Framework meshes also support animation data via the FRM_ANIMATION structure. The animation data is the usual set of key-framed rotations, translations, and scale values. To animate a mesh, the Framework can use a specified time value to interpolate key frames and build transformation matrices which, it writes back in the mesh’s frame hierarchy. Since a single mesh can have numerous animations (run, walk, jump, etc.) associated with it, animation data is stored in separate files. 

## Graphical User Interface Components

Beyond just simple text rendering, the Framework has a CFrmUserInterface class which simplifies the process of creating a graphical user interface for the samples. This interface allows users to display a sample heading in the top left corner of the screen, render specific blocks of text to the screen, and render overlays. The primary use of the graphical user interface is to implement a sample help screen. This help screen allows users to display pertinent sample variables along with their respective values, and also allows users to display a list of available sample keys along with their respective actions. 

The following example shows how the CFrmUserInterface class can be used in practice. 

```

CFrmFont Font;
if( FALSE == Font.Create( "Media\\Fonts\\Font.pak" ) )
       return FALSE;

CFrmPackedResource resource;
if( FALSE == resource.LoadFromFile( "Media\\Textures.pak" ) )
        return FALSE;

CFrmTexture* pTexture;
pTexture = resource.GetTexture( "Overlay" );

CFrmUserInterface UI;
if( FALSE == UI.Initialize( &Font, "Title" ) )
        return FALSE;

FLOAT32 fVariable   = 10.0f;
const CHAR* strVariable = "Some value"
UI.AddHelpFloatVariable( &fVariable, "Var1", "%3.1f" );
UI.AddHelpStringVariable( &strVariable, "Var2" );
UI.AddHelpKeyInfo( FRM_FONT_KEY_0, "Toggle Info Pane" );
UI.AddHelpKeyInfo( FRM_FONT_KEY_1, "Do something" );
UI.AddHelpKeyInfo( FRM_FONT_KEY_2, "Do something else" );
UI.AddOverlay( pTexture->m_hTextureHandle, -5, -5, 128, 64 );

 ...

UI.Render();

 ```

## Miscellaneous Utilities

In addition to the above, the Framework includes a collection of miscellaneous helper functions including the ability to get the current time, log messages, and take screen shots of the application. 
