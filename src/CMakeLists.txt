set(SPECKLE_LIB_DEPS)

# Boost
find_package(Boost
	COMPONENTS program_options
	REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND SPECKLE_LIB_DEPS ${Boost_LIBRARIES})

# TIFF
find_package(TIFF 4.0.0 REQUIRED)
include_directories(${TIFF_INCLUDE_DIRS})
list(APPEND SPECKLE_LIB_DEPS ${TIFF_LIBRARIES})

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS core highgui)
include_directories(${OpenCV_INCLUDE_DIRS})
list(APPEND SPECKLE_LIB_DEPS ${OpenCV_LIBRARIES})

# libfreenect
find_package(libfreenect)
if (libfreenect_FOUND)
	include_directories(${FREENECT_INCLUDE_DIRS})
	list(APPEND SPECKLE_LIB_DEPS ${FREENECT_LIBRARIES})
	# Add library directory
	# This is supposedly not needed for proper Find*.cmake modules
	link_directories(${FREENECT_LIBRARY_DIRS})
else()
	message(WARNING "libfreenect not found: will not build Kinect capture tools")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SPECKLE_SOURCES
	compute/ColourMap.cpp
	compute/ComputePipeline.cpp
	compute/CorrelationTime.cpp
	compute/SpatialWindow.cpp
	compute/Visualize.cpp)

if (libfreenect_FOUND)
	add_executable(capture
		tools/capture/capture.cpp
		tools/capture/KinectCapture.cpp)
	target_link_libraries(capture ${SPECKLE_LIB_DEPS})
endif()

add_executable(process
	tools/process/process.cpp
	${SPECKLE_SOURCES})
target_link_libraries(process ${SPECKLE_LIB_DEPS})

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

