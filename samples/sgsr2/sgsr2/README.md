# Snapdragon™ Game Super Resolution 2.0 Sample

## Overview

Snapdragon™ Game Super Resolution 2.0 (Snapdragon™ GSR 2.0 or SGSR 2.0) sample demonstrates how to integrate SGSR2.0 to do temporal upscaling in two or three GPU shader passes.

### Variants:

- **2-pass-fs  (recommended)**: Uses 2 fragment shaders, favors speed, and is often optimal for Mobile and VR/XR applications.
- **2-pass-cs**: Uses 2 compute shaders, favors speed, and is often optimal for Mobile and VR/XR applications.
- **3-pass-cs**: Uses 3 compute shaders, favors visuals, and is often optimal for Desktop/Console and higher-end Mobile applications.

## Build

### Dependencies

The following dependencies must be installed and the appropriate locations should be referenced in the `PATH` environment variable.

* Android SDK
* Andorid NDK
* Gradle
* CMake
* Android Studio

Before building any sample, run 03_BuildTools.bat at the root of the repository to ensure that all executables any sample might need are built.

At this point, building this sample APK is as simple as navigating to the `build\android` folder and on a command prompt using the following gradle command:

```

$ gradlew assembleRelease

```

Building a slightly-slower executable with asserts can be done with:

```

$ gradlew assembleDebug

```


This sample can also be easily imported to Android Studio and be used within the Android Studio ecosystem -- including building, deploying, and native code debugging.

To do this, open Android Studio and go to `File->New->Import Project...` and select the `build\android` folder as the source for the import. This will load the gradle configuration, at which point the sample can be used within Android Studio.

### Evaluation
Tapping the left part of screen switches SGSR 2-pass-fs/2-pass-cs/3-pass-cs/off.
Tapping the right part of screen switches from camera move to still to compare the visual quality better.


## Implementation Example
The core of the integration of SGSR2.0 is
```
VOID CSample::PrepareSGSR()
```
and
```
VOID CSample::DispatchSGSR()
```
