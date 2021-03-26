cmake_minimum_required(VERSION 3.6.0)

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue)

find_library(log-lib
        log)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file (GLOB_RECURSE GLESFrameworkSource ../../../../../src/*.cpp)
file (GLOB_RECURSE GLESFrameworkInclude ../../../../../include/*.h)

add_library(hello-fw SHARED
        ../../../src/Scene.cpp
        ${GLESFrameworkSource})

target_include_directories(hello-fw PRIVATE
        ../../../../../include/
		../../../../../include/Android
		../../../../../include/OpenGLES
		../../../../../include/Model)

target_link_libraries(hello-fw
        GLESv3
        android
        EGL
        native_app_glue
        ${log-lib})