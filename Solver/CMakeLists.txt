project(itkHeatFlow)
cmake_minimum_required(VERSION 2.8)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  )

# Include ITK
find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

# Include Eigen3
find_package( Eigen3 REQUIRED )
include_directories( ${EIGEN3_INCLUDE_DIR} )

add_executable( TestHeatFlow TestHeatFlow.cxx )
target_link_libraries( TestHeatFlow ${ITK_LIBRARIES} )

add_definitions( -D_SCL_SECURE_NO_WARNINGS )
