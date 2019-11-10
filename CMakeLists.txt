#
# project: melt
#
cmake_minimum_required(VERSION 2.8)
project(melt)
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)

include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

fips_setup()

add_definitions(-DSOKOL_NO_DEPRECATED)
if (FIPS_EMSCRIPTEN)
    add_definitions(-DSOKOL_GLES3)
    set(slang "glsl300es:glsl100")
else()
    add_definitions(-DSOKOL_GLCORE33)
    set(slang "glsl330")
endif()

add_subdirectory(sokol)

fips_begin_app(melt-sapp windowed)
  fips_files(melt.cc)
  fips_deps(sokol imgui)
fips_end_app()

fips_finish()