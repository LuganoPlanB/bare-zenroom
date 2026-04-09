cmake_minimum_required(VERSION 4.0)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)

project(bare_addon_zenroom C)

set(ZENROOM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../zenroom")
set(ZENROOM_LIBRARY "${ZENROOM_ROOT}/libzenroom.so")

add_bare_module(bare_addon)

target_sources(
  ${bare_addon}
  PRIVATE
    binding.c
)

target_include_directories(
  ${bare_addon}
  PRIVATE
    "${ZENROOM_ROOT}/src"
)

target_link_libraries(
  ${bare_addon}
  PRIVATE
    "${ZENROOM_LIBRARY}"
)

set_target_properties(
  ${bare_addon}
  PROPERTIES
    BUILD_RPATH "${ZENROOM_ROOT}"
)
