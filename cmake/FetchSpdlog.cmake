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

set_target_properties(spdlog PROPERTIES FOLDER "Dependencies")
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
