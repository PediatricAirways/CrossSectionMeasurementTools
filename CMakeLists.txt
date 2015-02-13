project(CrossSectionMeasurementTools)
cmake_minimum_required(VERSION 2.8)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  )

# Include Eigen3 for solving sparse linear systems
find_package( Eigen3 REQUIRED )
include_directories( ${EIGEN3_INCLUDE_DIR} )

unset(PYTHON_EXECUTABLE CACHE)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

set(MODULE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ComputeAirwayCrossSections)
add_subdirectory(ExcludeMouth)
add_subdirectory(ExtractCrossSections)
