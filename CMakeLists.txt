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

add_subdirectory(AirwayLaplaceSolution)
add_subdirectory(ComputeAirwayCrossSections)
add_subdirectory(ConvertPolyDataToImage)
add_subdirectory(ExcludeMouth)
add_subdirectory(ExtractCrossSections)
add_subdirectory(LaplaceBoundaryImageFilter)
add_subdirectory(LaplaceSolution)
add_subdirectory(Utilities)
