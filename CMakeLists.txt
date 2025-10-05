cmake_minimum_required(VERSION 3.10..4.69)


# Prohibit in-source tree builds.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are strictly prohibited.")
endif()

project(jmmt_tools2
        VERSION 0.1.0
        LANGUAGES C CXX
        DESCRIPTION "A full rewrite of jmmt_tools using mcolib"
        )

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(Policies)
include(jmmtProjectFuncs)

option(JMMT_USE_OPENSSL "Use OpenSSL library" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Third party dependencies
add_subdirectory(third_party/mcolib)
add_subdirectory(src/libjmmt)

add_subdirectory(src/tools/jmpak)
