# MIT License
#
# Copyright (c) 2020 Olivier Le Doeuff
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# ── CMAKE PROPERTIES ──

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(NETTCP_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(NETTCP_MAIN_PROJECT ON)
endif()

# ── PROJECT SETTINGS ──

# OPTIONS

set(NETTCP_VERSION_MAJOR 1)
set(NETTCP_VERSION_MINOR 3)
set(NETTCP_VERSION_PATCH 5)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE NETTCP_VERSION_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
if(NOT NETTCP_VERSION_TAG)
  set(NETTCP_VERSION_TAG 00000000)
endif(NOT NETTCP_VERSION_TAG)
set(NETTCP_VERSION_TAG_HEX 0x${NETTCP_VERSION_TAG})
set(NETTCP_VERSION ${NETTCP_VERSION_MAJOR}.${NETTCP_VERSION_MINOR}.${NETTCP_VERSION_PATCH} CACHE STRING "" FORCE)

# General
set(NETTCP_PROJECT "NetTcp" CACHE STRING "Project Name")
set(NETTCP_TARGET ${NETTCP_PROJECT} CACHE STRING "NetTcp library name")
set(NETTCP_VERSION ${NETTCP_VERSION_MAJOR}.${NETTCP_VERSION_MINOR}.${NETTCP_VERSION_PATCH} CACHE STRING "NetTcp current version, this is only decorative and will not configure any files" FORCE)

# CREATE PROJECT

project(${NETTCP_PROJECT} VERSION ${NETTCP_VERSION} LANGUAGES CXX)
if(NETTCP_MAIN_PROJECT)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(NETTCP_BUILD_SHARED OFF CACHE BOOL "Build as a shared library (ON) or as static (OFF)")
set(NETTCP_FOLDER_PREFIX ${NETTCP_PROJECT} CACHE STRING "Prefix folder for all NetTcp generated targets in generated project (only decorative)")

set(NETTCP_ENABLE_PCH ON CACHE BOOL
 "Enable precompile headers support for 'NetTcp'. \"
 Only work if CMake support 'target_precompile_headers'. \"
 This can speed up compilation time.")

set(NETTCP_ENABLE_QML ON CACHE BOOL "Embedded Debug Qml module for NetTcp class")
set(NETTCP_ENABLE_EXAMPLES OFF CACHE BOOL "Create or not a target for examples")
set(NETTCP_ENABLE_TESTS OFF CACHE BOOL "Create or not a target for tests")
set(NETTCP_ENABLE_INSTALL ${NETTCP_MAIN_PROJECT} CACHE BOOL "Enable NetTcp install")

# LOG OPTIONS

message(STATUS "---------------- NETTCP OPTIONS. ----------------")

#General
message(STATUS "NETTCP_PROJECT            : " ${NETTCP_PROJECT})
message(STATUS "NETTCP_TARGET             : " ${NETTCP_TARGET})
message(STATUS "NETTCP_VERSION            : " ${NETTCP_VERSION})
message(STATUS "NETTCP_VERSION_TAG        : " ${NETTCP_VERSION_TAG})
message(STATUS "NETTCP_BUILD_SHARED       : " ${NETTCP_BUILD_SHARED})
message(STATUS "NETTCP_ENABLE_EXAMPLES    : " ${NETTCP_ENABLE_EXAMPLES})
message(STATUS "NETTCP_ENABLE_TESTS       : " ${NETTCP_ENABLE_TESTS})
message(STATUS "NETTCP_FOLDER_PREFIX      : " ${NETTCP_FOLDER_PREFIX})
message(STATUS "NETTCP_ENABLE_QML         : " ${NETTCP_ENABLE_QML})
message(STATUS "NETTCP_ENABLE_PCH         : " ${NETTCP_ENABLE_PCH})

# Install
message(STATUS "NETTCP_ENABLE_INSTALL     : " ${NETTCP_ENABLE_INSTALL})
if(NETTCP_ENABLE_INSTALL)
  message(WARNING "NETTCP_ENABLE_INSTALL not yet supported")
  message(STATUS "NETTCP_INSTALL_PREFIX     : " ${NETTCP_INSTALL_PREFIX})
endif(NETTCP_ENABLE_INSTALL)

message(STATUS "---------------- DONE WITH OPTIONS. -----------------")

# ── DEPENDENCIES ──

find_package(Qt5 REQUIRED COMPONENTS
  Core
  Qml
  Network
)

if(NETTCP_ENABLE_QML)
  find_package(Qt5 REQUIRED COMPONENTS QuickCompiler)
endif()

include(cmake/Dependencies.cmake)
include(cmake/FetchSpdlog.cmake)
include(cmake/FetchQOlm.cmake)

if(NETTCP_ENABLE_TESTS)
  include(cmake/FetchGTest.cmake)
endif()

# ── NETTCP ──

if(NETTCP_ENABLE_QML)
  file(GLOB_RECURSE NETTCP_QML_SRCS qml/*.qml)

  foreach(SOURCE IN ITEMS ${NETTCP_QML_SRCS})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    file(RELATIVE_PATH SOURCE_PATH_REL ${CMAKE_CURRENT_SOURCE_DIR}/qml "${SOURCE_PATH}")
    string(REPLACE "/" "\\" GROUP_PATH "${SOURCE_PATH_REL}")
    source_group("Qml\\${GROUP_PATH}" FILES "${SOURCE}")
  endforeach()

  # Add qml to the qtquick compiler
  qtquick_compiler_add_resources(NETTCP_QML_RES ${CMAKE_CURRENT_SOURCE_DIR}/qml/NetTcp.qrc)

  # Trick to rerun cmake each time NetTcp.qrc is updated
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/NetTcp.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/NetTcp.qrc.junk
    )

  # Dont Run Moc on cpp file generated from qml
  foreach(NETTCP_CURRENT_QML_FILE ${NETTCP_QML_RES})
    set_property(SOURCE ${NETTCP_CURRENT_QML_FILE} PROPERTY SKIP_AUTOMOC ON)
    source_group("Qml\\QtQuickCompiler Files" FILES ${NETTCP_CURRENT_QML_FILE})
  endforeach()
endif()
# NETTCP_ENABLE_QML

set(NETTCP_SRCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NETTCP_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(NETTCP_PRIVATE_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include/Net/Tcp)

# NETTCP SOURCES
set(NETTCP_UTILS_SRCS
  ${NETTCP_SRCS_FOLDER}/Utils.cpp
  ${NETTCP_SRCS_FOLDER}/Version.cpp
  ${NETTCP_SRCS_FOLDER}/Logger.cpp
  )

set(NETTCP_API_SRCS
  ${NETTCP_SRCS_FOLDER}/Server.cpp
  ${NETTCP_SRCS_FOLDER}/Socket.cpp
  ${NETTCP_SRCS_FOLDER}/ServerWorker.cpp
  ${NETTCP_SRCS_FOLDER}/SocketWorker.cpp
  )

set(NETTCP_SRCS ${NETTCP_UTILS_SRCS}
  ${NETTCP_API_SRCS}
  ${NETTCP_QML_SRCS}
  ${NETTCP_QML_RES}
  )

set(NETTCP_UTILS_INCS
  ${NETTCP_PRIVATE_INCS_FOLDER}/NetTcp.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Export.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Property.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Utils.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Version.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Logger.hpp
  )

set(NETTCP_API_INCS
  ${NETTCP_PRIVATE_INCS_FOLDER}/IServer.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/ISocket.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Server.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/Socket.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/ServerWorker.hpp
  ${NETTCP_PRIVATE_INCS_FOLDER}/SocketWorker.hpp
  )

set(NETTCP_INCS ${NETTCP_UTILS_INCS}
  ${NETTCP_API_INCS}
  )

source_group("Tcp\\Utils" FILES ${NETTCP_UTILS_INCS})
source_group("Tcp" FILES ${NETTCP_API_INCS})

source_group("Tcp\\Utils" FILES ${NETTCP_UTILS_SRCS})
source_group("Tcp" FILES ${NETTCP_API_SRCS})

# NETTCP TARGET
if(NETTCP_BUILD_SHARED)

  add_library(${NETTCP_TARGET} SHARED ${NETTCP_SRCS} ${NETTCP_INCS})
  target_compile_definitions(${NETTCP_TARGET} PRIVATE "-DNETTCP_DLL_EXPORT")

else()

  add_library(${NETTCP_TARGET} STATIC  ${NETTCP_SRCS} ${NETTCP_INCS})
  target_compile_definitions(${NETTCP_TARGET} PUBLIC "-DNETTCP_STATIC")

endif()

# Set cxx 14 only on target and not the whole project
target_compile_features(${NETTCP_TARGET} PUBLIC cxx_std_14)

# Set include directory
target_include_directories(${NETTCP_TARGET} PUBLIC $<BUILD_INTERFACE:${NETTCP_INCS_FOLDER}>)

set_target_properties(${NETTCP_TARGET} PROPERTIES AUTOMOC TRUE)
if(NETTCP_ENABLE_QML)
  set_target_properties(${NETTCP_TARGET} PROPERTIES AUTORCC TRUE)
  target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_ENABLE_QML)
endif()

target_link_libraries(${NETTCP_TARGET} PUBLIC
  spdlog
  QOlm
  Qt::Core
  Qt::Qml
  Qt::Network
  )
set_target_properties(${NETTCP_TARGET} PROPERTIES FOLDER ${NETTCP_FOLDER_PREFIX})

if(NETTCP_ENABLE_PCH AND COMMAND target_precompile_headers)
  target_precompile_headers(${NETTCP_TARGET} PRIVATE ${NETTCP_PRIVATE_INCS_FOLDER}/Pch/Pch.hpp)
endif()

# Version
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_MAJOR=${NETTCP_VERSION_MAJOR})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_MINOR=${NETTCP_VERSION_MINOR})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_PATCH=${NETTCP_VERSION_PATCH})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_TAG=${NETTCP_VERSION_TAG})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_TAG_HEX=${NETTCP_VERSION_TAG_HEX})

# ── EXAMPLES ──

if(NETTCP_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

# ── NETTCP TESTS ──

if(NETTCP_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()


# ── CONFIGURATION ──

if(NETTCP_MAIN_PROJECT)
  include(cmake/PrintConfiguration.cmake)
endif()
