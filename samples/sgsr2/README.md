# Snapdragon™ Game Super Resolution 2.0 Sample

## Overview

Snapdragon™ Game Super Resolution 2.0 (Snapdragon™ GSR 2.0 or SGSR 2.0) sample demonstrates how to integrate SGSR2.0 to do temporal upscaling.

## Build

### Dependencies

The following dependencies must be installed and the appropriate locations should be referenced in the `PATH` environment variable.

* Android SDK
* Andorid NDK
* Gradle
* CMake
* Android Studio
* Other dependencies
   Run following commands at {root_of_this_repo}\GLES_Sample_Framework:
   ```
   .\deps.bat
   ```
   Make sure all the dependencies are cloned correctly. In my case, the submodules of adreno-gpu-opengl-es-code-sample-framework often failed to be cloned, to solve this try following commands repeatedly:
   ```
   cd {root_of_this_repo}\GLES_Sample_Framework\adreno-gpu-opengl-es-code-sample-framework
   git submodule update --init --recursive
   ```
   After all the submodules are cloned, run `.\deps.bat` again to build dependencies.

### Build
There are two samples demonstrate 3 variants of SGSR2.0.
- For SGSR2.0 2/3-pass compute shader variant, uses the sgsr2 sample
Open Android Studio and go to `File->New->Import Project...` and select the `adreno-gpu-opengl-es-code-sample-framework\samples\sgsr2\build\android` folder as the source for the import.

- For SGSR2.0 2-pass fragment shader variant, uses the sgsr2_fs sample
Open Android Studio and go to `File->New->Import Project...` and select the `adreno-gpu-opengl-es-code-sample-framework\samples\sgsr2_fs\build\android` folder as the source for the import.

### Evaluation
Tapping the left part of screen switches SGSR 2-pass/3-pass/off.
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
