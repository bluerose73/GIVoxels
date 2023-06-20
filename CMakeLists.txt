cmake_minimum_required(VERSION 3.10)
project(GIVoxels)

set(CMAKE_CXX_STANDARD 17)

# Make MSVC happy
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory(gi-voxels)
add_subdirectory(thirdparty)