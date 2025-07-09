cmake_minimum_required(VERSION 3.31)
project(Triumph LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Libraries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Libraries)

if (MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /utf-8)
    add_compile_definitions(STRICT NOMINMAX WIN32_EXTRA_LEAN VC_EXTRALEAN)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

if (WIN32)
    add_compile_definitions(PLATFORM_WIN32=1 VK_USE_PLATFORM_WIN32_KHR)
elseif (UNIX)
    add_compile_definitions(PLATFORM_UNIX=1)
endif ()

find_package(Vulkan REQUIRED)

if (NOT TARGET Vulkan::Headers)
    find_path(VULKAN_HPP_INCLUDE_DIR vulkan/vulkan.hpp
            HINTS ${Vulkan_INCLUDE_DIRS})
    if (NOT VULKAN_HPP_INCLUDE_DIR)
        message("vulkan.hpp not found!")
    endif ()
endif ()

add_subdirectory(Engine)
add_subdirectory(Game)