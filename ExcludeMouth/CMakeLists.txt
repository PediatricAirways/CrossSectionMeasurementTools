project(ExcludeSphereImageFilterTest)
cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME ExcludeSphereImageFilter)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK 4.7
  COMPONENTS
    ITKIOImageBase
    ITKVTK
 REQUIRED)
include(${ITK_USE_FILE})

add_executable( ExcludeSphereImageFilterTest ExcludeSphereImageFilterTest.cxx )
target_link_libraries( ExcludeSphereImageFilterTest ${ITK_LIBRARIES} )

# TODO - make a CLI module

add_definitions( -D_SCL_SECURE_NO_WARNINGS )