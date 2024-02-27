# Avoiding GMEM Loads (Unresolves)

<img width="400px" src="img/avoid_gmem_loads.png">

This sample shows what a GMEM Load is and why we need to avoid them. Removing unnecessary GMEM Loads can not only significantly improve the performance of your application but also reduce energy consumption.

For instructions on how to build this sample navigate to the [build](#build) section. To use the sample toggle the clear all/buffer by tapping the screen

Note: Newer drivers may optimize away unnecessary GMEM Loads, so you might not see them on recent drivers.

## What is a GMEM Load?

Graphics Memory (GMEM) is small, fast memory dedicated to the GPU.  The physical size of GMEM varies based on the GPU and is on the order of megabytes, depending on the device. In the device's tiled memory architecture, it contains a tile's color, depth and stencil attachments. Moving data from system memory to GMEM is called a GMEM Load or Unresolve. For more, see [Tiling Architecture](https://developer.qualcomm.com/docs/adreno-gpu/developer-guide/gpu/overview.html#tile-based-rendering).

## Why should we avoid GMEM Loads?

GMEM Loads increase the traffic between system memory and GPU, and usually negatively impacts performance while increasing power consumption.

## How do we identify GMEM Loads?

Use Snapdragon Profiler -- please see https://developer.qualcomm.com/software/snapdragon-profiler

From the screenshots below GPU time is 18.24ms without GMEM Loads. The frame time increases to 25.15 with GMEM Loads -- a 37% difference.
 

Here only the color and depth buffers are cleared on the FBO at the beginning of each frame:

<img src="img/withgmemload.png">

Here a clear all is executed on the FBO at the beginning of each frame:

<img src="img/withoutgmemload.png">

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
