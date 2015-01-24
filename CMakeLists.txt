project(vtkCrossSectionMeasurement)
cmake_minimum_required(VERSION 2.8)

# Include ITK
find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

# Include VTK
find_package( VTK REQUIRED )
vtk_module_config(CrossSection_VTK
  vtkRenderingOpenGL
  vtkInteractionStyle
  vtkRenderingVolumeOpenGL
  vtkRenderingFreeTypeOpenGL
  vtkFiltersGeneral
  vtkIOXML
  vtkImagingCore
)

# Add VTK
include_directories(${CrossSection_VTK_INCLUDE_DIRS})

# Add preprocessor definitions needed to use VTK.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${CrossSection_VTK_DEFINITIONS})

add_executable( TestCrossSectionMeasurement
  TestCrossSectionMeasurement.cxx
  vtkCrossSectionImageFilter.cxx
)
target_link_libraries( TestCrossSectionMeasurement ${ITK_LIBRARIES} ${CrossSection_VTK_LIBRARIES} )

add_definitions( -D_SCL_SECURE_NO_WARNINGS )