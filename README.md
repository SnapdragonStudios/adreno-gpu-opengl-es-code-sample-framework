

# Adreno™ GPU OpenGL ES Code Sample Framework

## Overview

This repository contains an OpenGL ES Framework for game and graphics developers to get up and running quickly for creating sample content and rapid prototyping. It is designed to be easy to build and have the basic building blocks needed for creating an Android APK with OpenGL ES functionality including an input system, as well as other helper utilities for loading resources, etc. This Framework has been extracted and is a subset of the [Adreno GPU SDK](https://developer.qualcomm.com/software/adreno-gpu-sdk/tools).

## How to use this Framework

For an overview of the key features and functionality of this Framework please visit the [Framework Overview](OVERVIEW.md) page. Then, visit the [Hello Framework](samples/hello-fw) sample page to get up and running with building, deploying and extending a basic OpenGL ES application.

## Building

This OpenGL ES Framework is designed to be compiled straight into your application's executable, for an example of building instructions specific to a simple use case, look at the [Hello Framework](samples/hello-fw) sample.

## Contributions
Thanks for your interest in contributing to the Adreno™ GPU OpenGL ES Code Sample Framework. Please read our [Contributions Page](CONTRIBUTING.md) for more information on contributing features or bug fixes. We look forward to your participation!

## License
Adreno™ GPU OpenGL ES Code Sample Framework is licensed under the BSD 3-clause “New” or “Revised” License. Check out the [LICENSE](LICENSE) for more details.

# Samples

## [Hello-Framework](samples/hello-fw)

<img width="200px" src="samples/hello-fw/img/screenshot.png">

Hello Framework sample demonstrates the most basic usage of the Framework to produce a native OpenGL ES application and it is designed to be small and simple and meant as a starting point for developers to expand its functionality.

## [Efficient MSAA](samples/msaa)

<img width="200px" src="samples/msaa/img/msaa.png">

This sample shows how to use the [EXT_multisampled_render_to_texture](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_multisampled_render_to_texture.txt) extension to utilize MSAA more efficiently on mobile platforms. By using this extension you will be able to continue to use MSAA and avoid a costly blit that would be necessary without it thus improving rendering performance.

## [Avoiding GMEM Loads](samples/avoid_gmem_loads)

<img width="400px" src="samples/avoid_gmem_loads/img/avoid_gmem_loads.png">

This sample shows what a GMEMLoad is and why we need to avoid them. Removing unnecessary GMEMLoads can not only significantly improve the performance of your application but also reduce energy consumption.

## [Reducing GMEM Stores](samples/reduce_gmem_stores)

<img width="400px" src="samples/reduce_gmem_stores/img/reduce_gmem_stores.png">

This sample shows what a GMEMStore is and why we need to reduce it. Removing unnecessary GMEM stores can not only significantly improve the performance of your application but also reduce energy consumption.

## [Shading Rate](samples/shading_rate)

<img width="800px" src="samples/shading_rate/img/shading_rate.png">

This sample shows how to use the [QCOM_shading_rate](https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_shading_rate.txt) extension to modify the shading rate per drawcall. Using this technique effectively can greatly increase performance with little to no visual quality degradation.
