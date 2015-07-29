cmake_minimum_required(VERSION 2.8)
project(SplitEpiglottisCrossSection)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(SplitEpiglottisCrossSection SplitEpiglottisCrossSection.cxx)
target_link_libraries(SplitEpiglottisCrossSection ${VTK_LIBRARIES})
