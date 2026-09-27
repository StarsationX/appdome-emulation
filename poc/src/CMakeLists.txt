cmake_minimum_required(VERSION 3.16)
project(loader LANGUAGES C CXX)

file(GLOB_RECURSE APPDOME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_library(loader SHARED ${APPDOME_SOURCES})
target_compile_features(loader PRIVATE cxx_std_17)
target_include_directories(loader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(loader PRIVATE z log)
