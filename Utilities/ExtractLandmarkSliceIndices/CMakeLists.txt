cmake_minimum_required(VERSION 2.8)
 
PROJECT(ExtractLandmarkSliceIndices)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(ExtractLandmarkSliceIndices MACOSX_BUNDLE ExtractLandmarkSliceIndices)
 
if(VTK_LIBRARIES)
  target_link_libraries(ExtractLandmarkSliceIndices ${VTK_LIBRARIES})
else()
  target_link_libraries(ExtractLandmarkSliceIndices vtkHybrid vtkWidgets)
endif()