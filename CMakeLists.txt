project(ComputeAirwayCrossSections)
cmake_minimum_required(VERSION 2.8)

unset(PYTHON_EXECUTABLE CACHE)

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

# Include ITK
include(${ITK_USE_FILE})

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

# Slicer doesn't enable ITK's VtkGlue module, so we add the include
# directory manually here.
load_cache( "${ITK_DIR}" ITK_SOURCE_DIR )
include_directories( ${ITK_SOURCE_DIR}/Modules/Bridge/VtkGlue/include )

# Add preprocessor definitions needed to use VTK.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${CrossSection_VTK_DEFINITIONS})

add_executable( TestCrossSectionMeasurement
  TestCrossSectionMeasurement.cxx
  vtkContourAtPointsFilter.cxx
)
target_link_libraries( TestCrossSectionMeasurement
  ${ITK_LIBRARIES}
  ${CrossSection_VTK_LIBRARIES}
)

SEMMacroBuildCLI(
  NAME ComputeAirwayCrossSections
  ADDITIONAL_SRCS
    vtkContourAtPointsFilter.cxx
  TARGET_LIBRARIES
    ${ITK_LIBRARIES}
    ${CrossSection_VTK_LIBRARIES}
  EXECUTABLE_ONLY
)

add_definitions( -D_SCL_SECURE_NO_WARNINGS )
