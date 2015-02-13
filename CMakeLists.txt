project(ComputeAirwayCrossSections)
cmake_minimum_required(VERSION 2.8)

unset(PYTHON_EXECUTABLE CACHE)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

set(MODULE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ComputeAirwayCrossSections)
add_subdirectory(ExcludeMouth)
add_subdirectory(ExtractCrossSections)
