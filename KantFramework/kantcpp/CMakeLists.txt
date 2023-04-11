cmake_minimum_required(VERSION 3.2)

project(kant-cpp)

#-------------------------------------------------------------------

if("${KANT_CPP_COMMON}" STREQUAL "")

    set(KANT_CPP_COMMON "1")

    message("PROJECT_SOURCE_DIR:              ${PROJECT_SOURCE_DIR}")

    #include("${PROJECT_SOURCE_DIR}/cmake/GenVersion.cmake")

    include("${PROJECT_SOURCE_DIR}/cmake/Common.cmake")

    #include("${PROJECT_SOURCE_DIR}/cmake/BuildTarsCpp.cmake")

    include("${PROJECT_SOURCE_DIR}/cmake/Thirdparty.cmake")

    #include("${PROJECT_SOURCE_DIR}/cmake/ParseAsm.cmake")

endif()

# set(major "")
# set(minor "")
# set(patch "")
# get_version(major minor patch)

# set(KANT_VERSION ${major}.${minor}.${patch})

#add_definitions与 #define功能一致
add_definitions(-DKANT_VERSION="${KANT_VERSION}")

message("KANT_VERSION:              ${KANT_VERSION}")

set(CLEAR_INCLUDE "clear-install.cmake")
FILE(WRITE ${CLEAR_INCLUDE} "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_INSTALL_PREFIX}/include)\n")
install(SCRIPT ${CLEAR_INCLUDE})

#-------------------------------------------------------------------

message("PROJECT_SOURCE_DIR:              ${PROJECT_SOURCE_DIR}")
include_directories(${PROJECT_SOURCE_DIR}/util/include)

#include_directories(${PROJECT_SOURCE_DIR}/servant)
#include_directories(${PROJECT_SOURCE_DIR}/servant/protocol)

IF(WIN32)
include_directories(${PROJECT_SOURCE_DIR}/util/src/epoll_windows)
ENDIF()

add_subdirectory(util)
add_subdirectory(tools)
add_subdirectory(servant)

IF (NOT ${ONLY_LIB})
# add_subdirectory(examples)
# add_subdirectory(unit-test)
ENDIF()


