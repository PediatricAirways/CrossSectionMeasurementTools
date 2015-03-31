project(LaplaceBoundaryImageFilter)
cmake_minimum_required(VERSION 2.8)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK 4.7 REQUIRED)
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
add_executable(LaplaceBoundaryImageFilter BoundaryFilterTest.cxx)
target_link_libraries(LaplaceBoundaryImageFilter
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)
