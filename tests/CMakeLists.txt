find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(NETTCP_TESTS_TARGET "${NETTCP_TARGET}Tests")

set(NETTCP_TESTS_SRCS
  Tests.cpp
  ServerTests.cpp
  SocketTests.cpp
  ${PROJECT_SOURCE_DIR}/examples/MySocketWorker.cpp
  ${PROJECT_SOURCE_DIR}/examples/MySocketWorker.hpp
  ${PROJECT_SOURCE_DIR}/examples/MySocket.cpp
  ${PROJECT_SOURCE_DIR}/examples/MySocket.hpp
  ${PROJECT_SOURCE_DIR}/examples/MyServer.cpp
  ${PROJECT_SOURCE_DIR}/examples/MyServer.hpp
)

message(STATUS "Add Test: ${NETTCP_TESTS_TARGET}")

add_executable(${NETTCP_TESTS_TARGET} ${NETTCP_TESTS_SRCS})
target_link_libraries(${NETTCP_TESTS_TARGET} PRIVATE
  ${NETTCP_TARGET}
  gtest
  Qt::Test
)
target_include_directories(${NETTCP_TESTS_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/examples)
set_target_properties(${NETTCP_TESTS_TARGET} PROPERTIES
  AUTOMOC TRUE
  FOLDER "Tests")

add_test(NAME ${NETTCP_TESTS_TARGET} COMMAND ${NETTCP_TESTS_TARGET})
