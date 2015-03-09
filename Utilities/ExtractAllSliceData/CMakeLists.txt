cmake_minimum_required(VERSION 2.8)
 
PROJECT(ExtractAllSliceData)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(ExtractAllSliceData MACOSX_BUNDLE ExtractAllSliceData)
 
if(VTK_LIBRARIES)
  target_link_libraries(ExtractAllSliceData ${VTK_LIBRARIES})
else()
  target_link_libraries(ExtractAllSliceData vtkHybrid vtkWidgets)
endif()