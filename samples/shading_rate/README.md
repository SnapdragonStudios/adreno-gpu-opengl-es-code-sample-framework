# Shading Rate

<img width="500px" src="img/shading_rate.png">

This sample shows how to use the [QCOM_shading_rate](https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_shading_rate.txt) extension to modify the shading rate per drawcall. Using this technique effectively can greatly increase performance with little to no visual quality degradation.

For instructions on how to build this sample navigate to the [build](#build) section.

## What is Shading Rate?

When the GPU renders and rasterizes objects into a surface, it will do so at a rate of one sample per pixel (assuming no multi-sampling is used although this concept can be applied to multi-sampled as well). This extension provides to the developer the ability to modify this shading rate to be coarser than a pixel. The supported resolutions of this extension are:

* GL_SHADING_RATE_1X1_PIXELS_QCOM (Default shading)
* GL_SHADING_RATE_1X2_PIXELS_QCOM
* GL_SHADING_RATE_2X1_PIXELS_QCOM
* GL_SHADING_RATE_2X2_PIXELS_QCOM
* GL_SHADING_RATE_4X2_PIXELS_QCOM
* GL_SHADING_RATE_4X4_PIXELS_QCOM

If, for example, GL_SHADING_RATE_4X4_PIXELS_QCOM is used. The result of the shading operation will be used to color an area of 4x4 pixels.

## Effective ways to modify shading rate

This extension can improve performance on heavily fragment bound drawcalls. As can be expected from reducing the shading rate, this can impact the visual quality of the rendered object if used innapropriately. Research on effective ways of modifying shading rate is active and several articles and talks can be found online on specific optimizations regarding shading rate modification. Some examples that can benefit from this technique are:

* Surfaces where the color variace is small
* Areas on a surface that do not require pixel accuracy for shading. This can be color targets that will be downscaled, if motion blur is enabled and there's significant velocity field changes, if depth of field is used there will be areas outside of the focus point that will be blurred.
* Effects like motion volumetric rendering where a portion of scene is processed at full shading rate and a portion can be processed at a reduced shading rate

It is worth noting that on mobile devices performance and power are tightly coupled. Improving performance using shading rate can also create gains on power consumption and reduce the thermal profile of your game ultimately increasing the users play time.

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
