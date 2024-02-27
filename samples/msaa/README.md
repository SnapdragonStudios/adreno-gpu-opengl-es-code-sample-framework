# Improving MSAA Performance

Using an anti-aliasing technique to reduce the artifacts of rasterization can improve visual quality. Due to the tiling architecture of mobile platforms, software based techniques can be prohibitively expensive -- hardware-accelerated MSAA can be visually effective and more power efficient. To take full advantage of this, an OpenGL ES extension is required to 'hint' the GPU, as detailed below.

This sample shows how to use the [EXT_multisampled_render_to_texture](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_multisampled_render_to_texture.txt) extension to utilize MSAA more efficiently on mobile platforms. Specifically, the extension allows you to use MSAA while avoiding a costly blit, improving rendering performance.

For instructions on how to build this sample navigate to the [build](#build) section.

## Problem Statement

The sample sets up a simple post processing pipeline as shown:

![Pipeline](img/pipeline.png)

Here, the scene is rendered on a 4xMSAA target and a post processing pipeline is applied to the output. To simplify this example, only one post FX is implemented in the sample: a simple vignetting shader. Ultimately the result is rendered to the back buffer. In order to accomplish this in OpenGL ES our first instinct is to create a pipeline as follows:

![Pipeline OpenGL ES Not Optimized](img/pipeline_gles_not_optimized.png)

This creates a 4xMSAA Render target which needs to blit into a single sample FBO to feed the post processing step. This workflow is inefficient for tiling architectures because the full 4xMSAA target needs to be resolved (GMEM Stored) and then blitted. Since the 4xMSAA target is only needed while rendering the scene, its full 4x content is not needed afterwards. [EXT_multisampled_render_to_texture](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_multisampled_render_to_texture.txt) allows us to create a 4xMSAA target that for scene rendering purposes behaves identically to the example above on a per-tile basis -- but content is resolved and filtered onto a single sample render target, avoiding the blit, as shown below:

![Pipeline OpenGL ES Optimized](img/pipeline_gles_optimized.png)

## Evaluating Behavior and Performance with Snapdragon Profiler

Using tools like [Snapdragon Profiler](https://developer.qualcomm.com/software/snapdragon-profiler) is essential to understand what is happening 'under the hood' while rendering to a mobile device. As shown in the figures below, Snapdragon Profiler's 'Rendering Stages' metric visualizes stage (Binning, GMEM Load, GMEM Store, Render, etc.) time information per surface, while displaying each surface's properties.

### Not Optimized
![Snapdragon Profiler Trace - Not Optimized](img/sdp_not_optimized.png)

### Optimized
![Snapdragon Profiler Trace - Optimized](img/sdp_optimized.png)

Important highlights:
* Optimized version does not perform a blit operation. This blit takes ~2.5ms in an Adreno 530 device.
* Rendering the scene on both versions is roughly equivalent -- since both use the same number of tiles, and they both render to a 4xMSAA target in GMEM.
* Performance improvement on this Adreno 530 device: ~3ms reduction.
* Even though this sample is not GPU bound, we can get accurate GPU timing information using [Snapdragon Profiler](https://developer.qualcomm.com/software/snapdragon-profiler).
* Side-note: We do not see any GMEM Load operations as we cleared the FBO's before use (see [Avoiding GMEM Loads](../avoid_gmem_loads) sample) and we do not resolve the depth buffer -- which further improves performance (see [Reducing GMEM Stores](../reduce_gmem_stores) sample).

## Implementation Details

All the code described here resides in the `Sample.cpp` file -- key functions are `CreateFBO()` and `Render()`. 

The sample provides one `CreateFBO()` function that takes an `isOptimized` parameter.  This shows how few changes are needed to port a standard pipeline (as described above) to the OpenGL ES extension. The key difference is the ability to setup normal texture FBO targets instead of render buffers, and constructing the FBO using [glFramebufferTexture2DMultisampleEXT](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_multisampled_render_to_texture.txt) API to hint the GPU to setup the MSAA FBO.

The `Render()` function shows the difference in pipeline execution: the optimized path does not require [glBlitFramebuffer](https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glBlitFramebuffer.xhtml) since the intermediate FBO is the input for the post effects pipeline.

## Build

### Dependencies

The following dependencies must be installed and the appropriate locations should be referenced in the `PATH` environment variable.

* Android SDK
* Andorid NDK
* Gradle
* CMake
* Android Studio

Before building any sample, run 03_BuildTools.bat at the root of the repository to ensure that all executables any sample might need are built.

Once the dependencies are installed, building this sample APK is as simple as navigating to the `build\android` folder and on a command prompt using the following gradle command:

```

$ gradlew assembleDebug

```

This sample can also be easily imported to Android Studio and be used within the Android Studio ecosystem -- including building, deploying, and native code debugging.

To do this, open Android Studio and go to `File->New->Import Project...` and select the `build\android` folder as the source for the import. This will load the gradle configuration, at which point the sample can be used within Android Studio.
