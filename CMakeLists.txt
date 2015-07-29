project(CrossSectionMeasurementTools)
cmake_minimum_required(VERSION 2.8)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  )

# Include Eigen3 for solving sparse linear systems
find_package( Eigen3 REQUIRED )
include_directories( ${EIGEN3_INCLUDE_DIR} )

# Locally defined ITK filters
include_directories( ITK )

unset(PYTHON_EXECUTABLE CACHE)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

# Put all executables in build/bin directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(MODULE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

add_subdirectory(ComputeLaplaceSolution)
add_subdirectory(ComputeCrossSections)
add_subdirectory(ComputeHeatContours)
add_subdirectory(ConvertDICOMToNRRD)
add_subdirectory(RemoveSphere)
add_subdirectory(ExtractCrossSections)
add_subdirectory(ComputeLBMBoundaries)
add_subdirectory(ResampleImage)
add_subdirectory(SplitEpiglottisCrossSection)
add_subdirectory(ThresholdLaplaceSolution)
add_subdirectory(Utilities)

# Unused utility to convert a polydata to a binary image
add_subdirectory(ConvertPolyDataToImage)
