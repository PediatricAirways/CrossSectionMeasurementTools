project(ExtractCrossSections)
cmake_minimum_required(VERSION 2.8)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

# Include VTK
find_package( VTK REQUIRED )
include(${VTK_USE_FILE})

### CLI module that extracts the cross sections closest to a set of
### query points and saves them as a poly data.
SEMMacroBuildCLI(
  NAME ExtractCrossSections
  TARGET_LIBRARIES
    ${VTK_LIBRARIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
)
