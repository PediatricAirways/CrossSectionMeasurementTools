project(ConvertPolyDataToImage)
cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
set(MODULE_NAME ConvertPolyDataToImage)

#-----------------------------------------------------------------------------
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(VTK)
include( ${VTK_USE_FILE} )

find_package(ITK)
include( ${ITK_USE_FILE} )

# Slicer doesn't enable ITK's VtkGlue module, so we add the include
# directory manually here.
load_cache( "${ITK_DIR}" READ_WITH_PREFIX My ITK_SOURCE_DIR )
include_directories( ${MyITK_SOURCE_DIR}/Modules/Bridge/VtkGlue/include )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES
        ${VTK_LIBRARIES} ${ITK_LIBRARIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
)
