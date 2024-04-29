include(cmake/CPM.cmake)

set(SPDLOG_REPOSITORY
    "https://github.com/gabime/spdlog"
    CACHE STRING "Repository of spdlog"
)
set(SPDLOG_TAG
    "592ea36a86a9c9049b433d9e44256d04333d8e52"
    CACHE STRING "Git tag/branch of spdlog"
)

CPMAddPackage(
  NAME spdlog
  GIT_REPOSITORY ${SPDLOG_REPOSITORY}
  GIT_TAG ${SPDLOG_TAG}
)

if(NOT TARGET spdlog)
  # Create an alias spdlog for other FetchSpdlog
  # dependencies to work that don't have the if(TARGET spdlog) condition
  add_library(spdlog INTERFACE)
  target_link_libraries(spdlog INTERFACE spdlog::spdlog)
  message(STATUS "spdlog target didn't exist, created an alias target")
endif()

if(TARGET spdlog)
  set_target_properties(spdlog PROPERTIES FOLDER "Dependencies")
endif()
