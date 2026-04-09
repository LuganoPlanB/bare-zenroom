cmake_minimum_required(VERSION 4.0)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)

project(bare_addon_zenroom C)

set(ZENROOM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/zenroom")
set(ZENROOM_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/bare-addon-zenroom")

if(WIN32)
  set(ZENROOM_RUNTIME_NAME "zenroom.dll")
  set(ZENROOM_LIBRARY "${ZENROOM_ROOT}/${ZENROOM_RUNTIME_NAME}")
  set(ZENROOM_IMPLIB "${ZENROOM_ROOT}/libzenroom_dll.lib")
elseif(APPLE)
  set(ZENROOM_RUNTIME_NAME "libzenroom.dylib")
  set(ZENROOM_LIBRARY "${ZENROOM_ROOT}/${ZENROOM_RUNTIME_NAME}")
else()
  set(ZENROOM_RUNTIME_NAME "libzenroom.so")
  set(ZENROOM_LIBRARY "${ZENROOM_ROOT}/${ZENROOM_RUNTIME_NAME}")
endif()

add_library(zenroom_local SHARED IMPORTED GLOBAL)

set_target_properties(
  zenroom_local
  PROPERTIES
    IMPORTED_LOCATION "${ZENROOM_LIBRARY}"
    IMPORTED_SONAME "${ZENROOM_RUNTIME_NAME}"
)

if(WIN32)
  set_target_properties(
    zenroom_local
    PROPERTIES
      IMPORTED_IMPLIB "${ZENROOM_IMPLIB}"
  )
endif()

add_bare_module(
  bare_addon
  INSTALL
    TARGET zenroom_local
)

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
    zenroom_local
)

add_custom_command(
  TARGET ${bare_addon}_module
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${ZENROOM_RUNTIME_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${ZENROOM_LIBRARY}"
          "${ZENROOM_RUNTIME_DIR}/${ZENROOM_RUNTIME_NAME}"
)
