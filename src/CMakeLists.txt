set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SPECKLE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/compute/ColourMap.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/compute/ComputePipeline.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/compute/CorrelationTime.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/compute/SpatialWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/compute/Visualize.cpp")

if (ENABLE_CAPTURE)
	add_subdirectory(tools/capture)
endif()

if (ENABLE_PROCESS)
	add_subdirectory(tools/process)
endif()

if (ENABLE_GUI)
	add_subdirectory(gui)
endif()
