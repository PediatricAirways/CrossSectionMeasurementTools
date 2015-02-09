cmake_minimum_required(VERSION 2.8)
 
PROJECT(SliceReader)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(SliceReader MACOSX_BUNDLE SliceReader)
 
if(VTK_LIBRARIES)
  target_link_libraries(SliceReader ${VTK_LIBRARIES})
else()
  target_link_libraries(SliceReader vtkHybrid vtkWidgets)
endif()