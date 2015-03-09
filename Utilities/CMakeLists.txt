project(Utilities)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${CrossSectionMeasurementTools_BINARY_DIR}/bin)

add_subdirectory(ExtractAllSliceData)
add_subdirectory(ExtractLandmarkSliceIndices)