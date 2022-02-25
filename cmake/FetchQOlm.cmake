include(cmake/CPM.cmake)

set(QOLM_REPOSITORY "https://github.com/OlivierLDff/QOlm.git" CACHE STRING "QOlm repository url")
set(QOLM_TAG master CACHE STRING "QOlm git tag")

CPMAddPackage(
  NAME qolm
  GIT_REPOSITORY ${QOLM_REPOSITORY}
  GIT_TAG ${QOLM_TAG}
)

