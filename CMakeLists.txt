#
#   NetTcp CMake build script
#
#   Copyright Olivier Le Doeuff 2020
#

# ┌──────────────────────────────────────────────────────────────────┐
# │                       CMAKE PROPERTIES                           │
# └──────────────────────────────────────────────────────────────────┘

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# DEPENDENCIES

set(OBJLISTMODEL_REPOSITORY "https://github.com/OlivierLDff/ObjectListModel" CACHE STRING "Repository of ObjectListModel")
set(OBJLISTMODEL_TAG "master" CACHE STRING "Git tag/branch of ObjectListModel")

# ┌──────────────────────────────────────────────────────────────────┐
# │                       PROJECT SETTINGS                           │
# └──────────────────────────────────────────────────────────────────┘

# OPTIONS

set(NETTCP_VERSION_MAJOR 1)
set(NETTCP_VERSION_MINOR 0)
set(NETTCP_VERSION_PATCH 0)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    EXECUTE_PROCESS(
        COMMAND git describe --always
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

set(NETTCP_BUILD_SHARED OFF CACHE BOOL "Build as a shared library (ON) or as static (OFF)")
set(NETTCP_FOLDER_PREFIX ${NETTCP_PROJECT} CACHE STRING "Prefix folder for all NetTcp generated targets in generated project (only decorative)")

set(NETTCP_ENABLE_QML ON CACHE BOOL "Embedded Debug Qml module for NetTcp class")

# Install
set(NETTCP_ENABLE_EXAMPLES OFF CACHE BOOL "Create or not a target for examples")

# Install
set(NETTCP_ENABLE_INSTALL OFF CACHE BOOL "Create or not a target for install")
set(NETTCP_INSTALL_PREFIX ${NETTCP_PROJECT} CACHE STRING "Prefix for all NetTcp headers in the install folder")

# CREATE PROJECT

project(${NETTCP_PROJECT} VERSION ${NETTCP_VERSION} LANGUAGES C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# LOG OPTIONS

message(STATUS "---------------- NETTCP OPTIONS. ----------------")

#General
message(STATUS "NETTCP_PROJECT            : " ${NETTCP_PROJECT})
message(STATUS "NETTCP_TARGET             : " ${NETTCP_TARGET})
message(STATUS "NETTCP_VERSION            : " ${NETTCP_VERSION})
message(STATUS "NETTCP_VERSION_TAG        : " ${NETTCP_VERSION_TAG})
message(STATUS "NETTCP_BUILD_SHARED       : " ${NETTCP_BUILD_SHARED})
message(STATUS "NETTCP_ENABLE_EXAMPLES    : " ${NETTCP_ENABLE_EXAMPLES})
message(STATUS "NETTCP_FOLDER_PREFIX      : " ${NETTCP_FOLDER_PREFIX})
message(STATUS "NETTCP_ENABLE_QML         : " ${NETTCP_ENABLE_QML})

# Install
message(STATUS "NETTCP_ENABLE_INSTALL     : " ${NETTCP_ENABLE_INSTALL})
if(NETTCP_ENABLE_INSTALL)
message(STATUS "NETTCP_INSTALL_PREFIX     : " ${NETTCP_INSTALL_PREFIX})
endif(NETTCP_ENABLE_INSTALL)

message(STATUS "---------------- DONE WITH OPTIONS. -----------------")

# ┌──────────────────────────────────────────────────────────────────┐
# │                         QT5 CMAKE                                │
# └──────────────────────────────────────────────────────────────────┘

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Qml CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)

if(NETTCP_ENABLE_QML)
    find_package(Qt5QuickCompiler CONFIG REQUIRED)
endif() # NETTCP_ENABLE_QML

# ┌──────────────────────────────────────────────────────────────────┐
# │                       DEPENDENCIES                               │
# └──────────────────────────────────────────────────────────────────┘

include(FetchContent)

# ObjectListModel
FetchContent_Declare(
    ObjectListModel
    GIT_REPOSITORY ${OBJLISTMODEL_REPOSITORY}
    GIT_TAG        ${OBJLISTMODEL_TAG}
)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET ON)

# Make all dependencies avaiables
FetchContent_MakeAvailable(ObjectListModel)

# ┌──────────────────────────────────────────────────────────────────┐
# │                          NETTCP                                  │
# └──────────────────────────────────────────────────────────────────┘

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
      ${CMAKE_CURRENT_BINARY_DIR}/qml.junk
    )

    # Dont Run Moc on cpp file generated from qml
    foreach(NETTCP_CURRENT_QML_FILE ${NETTCP_QML_RES})
        set_property(SOURCE ${NETTCP_CURRENT_QML_FILE} PROPERTY SKIP_AUTOMOC ON)
        source_group("Qml\\QtQuickCompiler Files" FILES ${NETTCP_CURRENT_QML_FILE})
    endforeach()
endif() # NETTCP_ENABLE_QML

set(NETTCP_SRCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NETTCP_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(NETTCP_PRIVATE_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include/Net/Tcp)

# NETTCP SOURCES
set(NETTCP_UTILS_SRCS
    ${NETTCP_SRCS_FOLDER}/Utils.cpp
    ${NETTCP_SRCS_FOLDER}/Version.cpp
)

set(NETTCP_API_SRCS
    ${NETTCP_SRCS_FOLDER}/AbstractServer.cpp
    ${NETTCP_SRCS_FOLDER}/AbstractSocket.cpp
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
)

set(NETTCP_API_INCS
    ${NETTCP_PRIVATE_INCS_FOLDER}/IServer.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/ISocket.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/AbstractServer.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/AbstractSocket.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/Server.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/Socket.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/ServerWorker.hpp
    ${NETTCP_PRIVATE_INCS_FOLDER}/SocketWorker.hpp
)
# endif(NETTCP_ENABLE_FLATBUFFERS)

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

else(NETTCP_BUILD_SHARED)

    add_library(${NETTCP_TARGET} STATIC  ${NETTCP_SRCS} ${NETTCP_INCS})
    target_compile_definitions(${NETTCP_TARGET} PUBLIC "-DNETTCP_STATIC")

endif(NETTCP_BUILD_SHARED)

# Set cxx 14 only on target and not the whole project
target_compile_features(${NETTCP_TARGET} PUBLIC cxx_std_14)

# Set include directory
target_include_directories(${NETTCP_TARGET} PUBLIC $<BUILD_INTERFACE:${NETTCP_INCS_FOLDER}> $<INSTALL_INTERFACE:${NETTCP_INSTALL_PREFIX}>)

# Avoid libnetcode.io on darwin for example.
set_target_properties(${NETTCP_TARGET} PROPERTIES PREFIX "")

# Link libraries
target_link_libraries(${NETTCP_TARGET} ObjectListModel)

set_target_properties(${NETTCP_TARGET} PROPERTIES AUTOMOC TRUE)
if(NETTCP_ENABLE_QML)
    set_target_properties(${NETTCP_TARGET} PROPERTIES AUTORCC TRUE)
endif() # NETTCP_ENABLE_QML
qt5_use_modules(${NETTCP_TARGET} Core Qml Network)

set_target_properties(${NETTCP_TARGET} PROPERTIES FOLDER ${NETTCP_FOLDER_PREFIX})

# Version
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_MAJOR=${NETTCP_VERSION_MAJOR})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_MINOR=${NETTCP_VERSION_MINOR})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_PATCH=${NETTCP_VERSION_PATCH})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_TAG=${NETTCP_VERSION_TAG})
target_compile_definitions(${NETTCP_TARGET} PRIVATE -DNETTCP_VERSION_TAG_HEX=${NETTCP_VERSION_TAG_HEX})

# ┌──────────────────────────────────────────────────────────────────┐
# │                           TESTS                                  │
# └──────────────────────────────────────────────────────────────────┘

# ┌──────────────────────────────────────────────────────────────────┐
# │                         EXAMPLES                                 │
# └──────────────────────────────────────────────────────────────────┘

if(NETTCP_ENABLE_EXAMPLES)

    set(NETTCP_EXAMPLE1_TARGET ${NETTCP_TARGET}_EchoClientServer)
    message(STATUS "Add Example ${NETTCP_EXAMPLE1_TARGET}")
    add_executable(${NETTCP_EXAMPLE1_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/examples/EchoClientServer.cpp")
    target_link_libraries(${NETTCP_EXAMPLE1_TARGET} NetTcp)
    set_target_properties(${NETTCP_EXAMPLE1_TARGET} PROPERTIES AUTOMOC TRUE)
    set_target_properties(${NETTCP_EXAMPLE1_TARGET} PROPERTIES FOLDER "${NETTCP_FOLDER_PREFIX}/Examples")

endif(NETTCP_ENABLE_EXAMPLES)

# ┌──────────────────────────────────────────────────────────────────┐
# │                         INSTALL                                  │
# └──────────────────────────────────────────────────────────────────┘

if(NETTCP_ENABLE_INSTALL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CInstallLists.txt)
endif(NETTCP_ENABLE_INSTALL)
