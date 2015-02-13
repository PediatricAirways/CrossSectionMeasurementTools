project(ExcludeSphereImageFilterTest)
cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME ExcludeMouth)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package(ITK 4.7
  COMPONENTS
    ITKIOImageBase
 REQUIRED)
include(${ITK_USE_FILE})

add_executable( ExcludeSphereImageFilterTest ExcludeSphereImageFilterTest.cxx )
target_link_libraries( ExcludeSphereImageFilterTest ${ITK_LIBRARIES} )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES ${ITK_DIRECTORIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
  )


add_definitions( -D_SCL_SECURE_NO_WARNINGS )