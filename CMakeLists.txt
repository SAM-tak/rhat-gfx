# The native part of rhat-gfx: SDL3, the C layers behind the gfx and audio
# modules (src/gfx over SDL_GPU, src/audio over SDL's audio), and the shader
# compiler (src/shaderc). The layers are the goose_gfx ones, unchanged; the
# R^ modules reach them through the shim in src/shim.

cmake_minimum_required(VERSION 3.20)
project(rhat_gfx C)

set(RHAT_GFX_SDL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL" CACHE PATH "The SDL source tree")
if(NOT EXISTS "${RHAT_GFX_SDL}/CMakeLists.txt")
    message(FATAL_ERROR "SDL not found: run git submodule update --init")
endif()

# Static only. SDL_GPU draws through neither the 2D renderer nor OpenGL, and
# nothing uses the camera, so those are left out.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${RHAT_GFX_SDL}" SDL EXCLUDE_FROM_ALL)

add_library(rhat_gfx STATIC
    src/gfx/gfx_device.c
    src/gfx/gfx_resources.c
    src/gfx/gfx_pipeline.c
    src/gfx/gfx_draw.c)
target_link_libraries(rhat_gfx PUBLIC SDL3::SDL3-static)

add_library(rhat_audio STATIC src/audio/audio.c)
target_link_libraries(rhat_audio PUBLIC SDL3::SDL3-static)

# GLSL in, a gfx shader blob out; cute_spirv is one header compiled once here.
add_library(rhat_shaderc STATIC src/shaderc/shaderc.c)
target_include_directories(rhat_shaderc PUBLIC src/shaderc
    PRIVATE src/gfx third_party/cute_spirv)

foreach(t IN ITEMS rhat_gfx rhat_audio rhat_shaderc)
    set_target_properties(${t} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
endforeach()
if(MSVC)
    foreach(t IN ITEMS rhat_gfx rhat_audio)
        target_compile_options(${t} PRIVATE /W4 /utf-8)
    endforeach()
    foreach(t IN ITEMS rhat_gfx rhat_audio rhat_shaderc)
        target_compile_definitions(${t} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endforeach()
else()
    foreach(t IN ITEMS rhat_gfx rhat_audio)
        target_compile_options(${t} PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
    endforeach()
    target_link_libraries(rhat_shaderc PUBLIC m)
endif()

# What an R^ program links: the shim over the three, and through it SDL.
add_library(rhat_gfx_shim STATIC src/shim/rhat_gfx.c)
target_include_directories(rhat_gfx_shim PUBLIC src/shim PRIVATE src/gfx src/audio)
target_link_libraries(rhat_gfx_shim PUBLIC rhat_gfx rhat_audio rhat_shaderc)
set_target_properties(rhat_gfx_shim PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
if(MSVC)
    target_compile_options(rhat_gfx_shim PRIVATE /W4 /utf-8)
    target_compile_definitions(rhat_gfx_shim PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(rhat_gfx_shim PRIVATE -Wall -Wextra)
endif()
