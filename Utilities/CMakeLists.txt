project(Utilities)
cmake_minimum_required(VERSION 2.8)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  )

add_subdirectory(ExtractAllSliceData)
add_subdirectory(ExtractLandmarkSliceIndices)