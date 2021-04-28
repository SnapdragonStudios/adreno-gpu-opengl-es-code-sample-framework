# Reducing GMEM Stores (Resolves)

<img width="400px" src="img/reduce_gmem_stores.png">

This sample shows what a GMEMStore is and why we need to reduce it. Removing unnecessary GMEM stores can not only significantly improve the performance of your application but also reduce energy consumption.

For instructions on how to build this sample navigate to the [build](#build) section. To use this sample toggle the all/color attachments by clicking the screen.

Note: Newer driver may try to optimize when detecting potential unnecessary GMEM Store. you might not see the GMEM depth/stencil stores on recent drivers

## What is a GMEM Store?

Graphics Memory (GMEM) is a dedicated fast memory on the GPU. It is fast and small. The physical size of GMEM varies based on the GPU and is about 1mb depending on the device. For tiled rendering architecture, it fits a tile's color attachments, depth and stencil attachments. The need to store data from GMEM to system memory is a GMEM Store or Resolve. Please see the documentation in [Tiling Architecture](https://developer.qualcomm.com/docs/adreno-gpu/developer-guide/gpu/overview.html#tile-based-rendering).

## Why we need to reduce it

A GMEM store increases the traffic between system memory and GPU, it usually has negative impacts on performance, more importantly it could increase power.

## How to Identify a GMEM Store

Use Snapdragon Profiler. Please see the tools page for snapdragon profiler.

From the screenshots below GPU time is 18.33ms with GMEM depth stencil store. The frame time decreases to 11.93 without GMEM Depth stencil store, 50% difference.
 

This screenshot is taken when there is no depth/stencil attachment for the FBO. This FBO's color attachment is used as a texture in the following rendering. And it is unnecessary to attach depth and stencil which will save extra GMEM store.

<img src="img/lessstore.png">

This screenshot is taken when there is depth and stencil attachment for the FBO. The depth and stencil attachment is useless for the rendering, however it still adds extra GMEM store.

<img src="img/hasstore.png">

## Build

### Dependencies

The following dependencies must be installed and the appropriate locations should be referenced in the `PATH` environment variable.

* Android SDK
* Andorid NDK
* Gradle
* CMake
* Android Studio

Once the dependencies are installed, building this sample APK is as simple as navigating to the `build\android` folder and on a command prompt using the following gradle command:

```

$ gradle assembleDebug

```

This sample can also be easily imported to Android Studio and be used within the Android Studio ecosystem including building, deploying, and native code debugging.

To do this, open Android Studio and go to `File->New->Import Project...` and select the `build\android` folder as the source for the import. This will load up the gradle configuration and once finalized, the sample can be used within Android Studio.
