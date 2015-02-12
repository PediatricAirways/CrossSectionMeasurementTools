cmake_minimum_required(VERSION 2.8)
 

#-----------------------------------------------------------------------------
project(ExcludeSphereImageFilterTest)


set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  )

# Include ITK
find_package( Slicer REQUIRED )
include( ${Slicer_USE_FILE} )


add_executable( ExcludeSphereImageFilterTest ExcludeSphereImageFilterTest.cxx )
target_link_libraries( ExcludeSphereImageFilterTest ${ITK_LIBRARIES} )

add_definitions( -D_SCL_SECURE_NO_WARNINGS )