# Hello Framework Sample

![Screenshot](img/screenshot.png)

## Overview

Hello Framework sample demonstrates the most basic usage of the Framework to produce a native OpenGL ES application.  It is designed to be a small and simple example for developers to expand on.

We recommend this sample as a starting point for other applications using this Framework. 

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

To do this, open Android Studio and go to `File->New->Import Project...` and select the `hello_fw\build\android` folder as the source for the import. This will load the gradle configuration, at which point the sample can be used within Android Studio.
