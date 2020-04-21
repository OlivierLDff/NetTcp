cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(NETTCP_EXAMPLE_LIB ${NETTCP_TARGET}_MyClientServer)

add_library(${NETTCP_EXAMPLE_LIB} MySocketWorker.cpp
    MySocketWorker.hpp
    MySocket.cpp
    MySocket.hpp
    MyServer.cpp
    MyServer.hpp)
target_link_libraries(${NETTCP_EXAMPLE_LIB} ${NETTCP_TARGET})
set_target_properties(${NETTCP_EXAMPLE_LIB} PROPERTIES AUTOMOC TRUE)
set_target_properties(${NETTCP_EXAMPLE_LIB} PROPERTIES FOLDER "${NETTCP_FOLDER_PREFIX}/Examples")
target_include_directories(${NETTCP_EXAMPLE_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(NETTCP_EXAMPLE1_TARGET ${NETTCP_TARGET}_EchoClientServer)
message(STATUS "Add Example ${NETTCP_EXAMPLE1_TARGET}")
add_executable(${NETTCP_EXAMPLE1_TARGET} EchoClientServer.cpp)
target_link_libraries(${NETTCP_EXAMPLE1_TARGET} ${NETTCP_EXAMPLE_LIB})
set_target_properties(${NETTCP_EXAMPLE1_TARGET} PROPERTIES FOLDER "${NETTCP_FOLDER_PREFIX}/Examples")

set(NETTCP_EXAMPLE2_TARGET ${NETTCP_TARGET}_EchoServer)
message(STATUS "Add Example ${NETTCP_EXAMPLE2_TARGET}")
add_executable(${NETTCP_EXAMPLE2_TARGET} EchoServer.cpp)
target_link_libraries(${NETTCP_EXAMPLE2_TARGET} ${NETTCP_EXAMPLE_LIB})
set_target_properties(${NETTCP_EXAMPLE2_TARGET} PROPERTIES FOLDER "${NETTCP_FOLDER_PREFIX}/Examples")

set(NETTCP_EXAMPLE3_TARGET ${NETTCP_TARGET}_EchoClient)
message(STATUS "Add Example ${NETTCP_EXAMPLE3_TARGET}")
add_executable(${NETTCP_EXAMPLE3_TARGET} EchoClient.cpp)
target_link_libraries(${NETTCP_EXAMPLE3_TARGET} ${NETTCP_EXAMPLE_LIB})
set_target_properties(${NETTCP_EXAMPLE3_TARGET} PROPERTIES FOLDER "${NETTCP_FOLDER_PREFIX}/Examples")