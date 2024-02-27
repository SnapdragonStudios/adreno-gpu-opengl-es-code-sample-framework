cmake_minimum_required(VERSION 3.6.0)

project(amfe_power_saving)

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue)

find_library(log-lib
        log)

# now build app's shared lib
set(CMAKE_CXX_FLAGS_DEBUG "-DADRENO_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file (GLOB_RECURSE GLESFrameworkSource ../../../../../src/*.cpp)
file (GLOB_RECURSE GLESFrameworkInclude ../../../../../include/*.h)

add_library(amfe_power_saving SHARED
        ../../../src/Scene.cpp
        ${GLESFrameworkSource})

add_compile_options("-Wno-unknown-warning-option")#suppress third-party submodule'd code warnings; ktx unfortunately emits "warning: unknown warning group '-Wmisleading-indentation', ignored [-Wunknown-warning-option]" --Nov, 2023
target_include_directories(amfe_power_saving PRIVATE
        ../../../../../include/
        ../../../../../framework/external
        ../../../../../framework/external/KTX-Software/include
        ../../../../../include/Android
        ../../../../../include/OpenGLES
        ../../../../../include/Model)
        
# point to the directory with ktx's CMakeLists.txt, so that cmake builds all the code in this directory
include_directories(../../../../../framework/external/KTX-Software)
add_subdirectory   (../../../../../framework/external/KTX-Software ../../../../../framework/external/KTX-Software)

target_link_libraries(amfe_power_saving
        GLESv3
        ktx_read
        android
        EGL
        native_app_glue
        ${log-lib})
		

# Copy required models to local folders
#
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../cmake/ModelPackager.cmake)

# Museum GLTF
add_gltf(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../assets/scenes/Museum/Museum)

#
# Convert and copy textures to local folders
#
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../cmake/TexturePackager.cmake)

# Museum Textures
add_textures_from_path(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../assets/scenes/Museum/Textures)

# Default Textures
add_textures_from_path(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../assets/textures)

# Copy from Media directory to final assets directory (that is packaged into the final apk)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Media/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/assets/)

# Font Texture
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../assets/fonts/gles/Tuffy16.pak DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/assets/)