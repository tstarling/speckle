cmake_minimum_required(VERSION 2.8)

# Options
set(ENABLE_CAPTURE TRUE CACHE BOOL "Enable the Kinect capture tool")
set(ENABLE_PROCESS TRUE CACHE BOOL "Enable the command line processing tool")
set(ENABLE_GUI TRUE CACHE BOOL "Enable the Qt GUI")
set(ENABLE_TEST TRUE CACHE BOOL "Enable self-testing")

# Global settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Boost
find_package(Boost
	COMPONENTS program_options
	REQUIRED)

function (UseBoost target)
	target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIRS})
	target_link_libraries(${target} ${Boost_LIBRARIES})
endfunction()

# TIFF
find_package(TIFF 4.0.0)

function (UseTiff target)
	if (NOT TIFF_FOUND)
		message(FATAL_ERROR "TIFF is required for target ${target}")
	endif()
	target_include_directories(${target} PRIVATE ${TIFF_INCLUDE_DIRS})
	target_link_libraries(${target} ${TIFF_LIBRARIES})
endfunction()

# OpenCV
find_package(OpenCV COMPONENTS core highgui)

function (UseOpenCV target)
	if (NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV is required for target ${target}")
	endif()
	target_include_directories(${target} PRIVATE ${OpenCV_INCLUDE_DIRS})
	target_link_libraries(${target} ${OpenCV_LIBRARIES})
endfunction()

# libfreenect
find_package(libfreenect)

if (libfreenect_FOUND)
	# Add library directory
	# This is supposedly not needed for proper Find*.cmake modules
	link_directories(${FREENECT_LIBRARY_DIRS})
endif()

function (UseFreenect target)
	if (NOT libfreenect_FOUND)
		message(FATAL_ERROR "libfreenect is required for target ${target}")
	endif()
	target_include_directories(${target} PRIVATE ${FREENECT_INCLUDE_DIRS})
	target_link_libraries(${target} ${FREENECT_LIBRARIES})
endfunction()

# Qt
find_package(Qt5Widgets)

function (UseQt target)
	if (NOT Qt5Widgets_FOUND)
		message(FATAL_ERROR "Qt5 is required for target ${target}")
	endif()
	target_link_libraries(${target} Qt5::Widgets)
	set(CMAKE_AUTOMOC ON)
endfunction()

# Threads
find_package(Threads)

function (UseThreads target)
	target_link_libraries(${target} ${CMAKE_THREAD_LIBS_INIT})
endfunction()

# libspeckle
add_library(speckle
	src/compute/ColourMap.cpp
	src/compute/ComputePipeline.cpp
	src/compute/CorrelationTime.cpp
	src/compute/SpatialWindow.cpp
	src/compute/Visualize.cpp)

function (UseSpeckle target)
	target_link_libraries(${target} speckle)
endfunction()

# capture
if (ENABLE_CAPTURE)
	add_executable(capture
		src/tools/capture/capture.cpp
		src/tools/capture/KinectCapture.cpp)
	UseBoost(capture)
	UseFreenect(capture)
	UseTiff(capture)
	UseOpenCV(capture)
endif()

# process
if (ENABLE_PROCESS)
	add_executable(process src/tools/process/process.cpp)
	UseBoost(process)
	UseTiff(process)
	UseOpenCV(process)
	UseSpeckle(process)
endif()

# gui
if (ENABLE_GUI)
	add_executable(gui
		src/gui/gui.cpp
		src/gui/MainWindow.cpp)

	UseOpenCV(gui)
	UseQt(gui)
	UseFreenect(gui)
	UseThreads(gui)
	UseSpeckle(gui)
endif()

# test
if (ENABLE_TEST)
	enable_testing()
	add_executable(test-runner test/test-runner.cpp)
	UseOpenCV(test-runner)
	UseSpeckle(test-runner)

	add_test(
		NAME SpatialWindow
		COMMAND $<TARGET_FILE:test-runner>
			SpatialWindow ${CMAKE_CURRENT_SOURCE_DIR}/test/SpatialWindow.tsv)

	add_test(
		NAME CorrelationTime
		COMMAND $<TARGET_FILE:test-runner>
			CorrelationTime ${CMAKE_CURRENT_SOURCE_DIR}/test/CorrelationTime.tsv)
endif()


