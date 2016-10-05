cmake_minimum_required(VERSION 2.8)

set(ENABLE_CAPTURE TRUE CACHE BOOL "Enable the Kinect capture tool")
set(ENABLE_PROCESS TRUE CACHE BOOL "Enable the command line processing tool")
set(ENABLE_GUI TRUE CACHE BOOL "Enable the Qt GUI")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_subdirectory(src)

