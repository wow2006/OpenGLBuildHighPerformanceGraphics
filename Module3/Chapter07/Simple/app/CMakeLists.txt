cmake_minimum_required(VERSION 3.4.1)
project(gl3jni C CXX)

include(AndroidNdkModules)

add_library(
        ${PROJECT_NAME}
        SHARED
        src/main/cpp/main_simple.cpp
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        GLESv2
        EGL
        log
        android
)

target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        -W -Wall
)

target_compile_features(
        ${PROJECT_NAME}
        PRIVATE
        cxx_constexpr
        cxx_nullptr
        cxx_auto_type
)