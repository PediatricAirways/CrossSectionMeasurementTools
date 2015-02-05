cmake_minimum_required(VERSION 2.8)
 
PROJECT(SliceToCsv)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(SliceToCsv MACOSX_BUNDLE SliceToCsv)
 
if(VTK_LIBRARIES)
  target_link_libraries(SliceToCsv ${VTK_LIBRARIES})
else()
  target_link_libraries(SliceToCsv vtkHybrid vtkWidgets)
endif()