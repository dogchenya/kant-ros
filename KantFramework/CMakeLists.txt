cmake_minimum_required(VERSION 3.14)

project(framework)

set(CMAKE_VERBOSE_MAKEFILE off)
set(KANT_CPP_COMMON "1")

#include("${PROJECT_SOURCE_DIR}/kantcpp/cmake/GenVersion.cmake")

include("${PROJECT_SOURCE_DIR}/kantcpp/cmake/Common.cmake")

include("${PROJECT_SOURCE_DIR}/kantcpp/cmake/BuildKantCpp.cmake")

include("${PROJECT_SOURCE_DIR}/kantcpp/cmake/Thirdparty.cmake")

include("${PROJECT_SOURCE_DIR}/kantcpp/cmake/ParseAsm.cmake")

add_subdirectory(kantcpp)

set(major "3")
set(minor "0")
set(patch "6")
set(FRAMEWORK_VERSION ${major}.${minor}.${patch})
add_definitions(-DFRAMEWORK_VERSION="${FRAMEWORK_VERSION}")
add_definitions(-DKANT_VERSION="${KANT_VERSION}")

message("FRAMEWORK_VERSION:         ${FRAMEWORK_VERSION}")
message("PROJECT_SOURCE_DIR:         ${PROJECT_SOURCE_DIR}")
message("----------------------------------------------------")

include_directories(${servant_SOURCE_DIR})

set(BUILD_NODE node)

if(WIN32)
    set(KANT_INSTALL_PATH c:\\tars-install)
else(WIN32)
    set(KANT_INSTALL_PATH /usr/local/app)
endif(WIN32)

message("KANT_INSTALL_PATH:         ${KANT_INSTALL_PATH}")

# 与function类似，供其他地方调用
macro(complice_module MODULE)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${BUILD_NODE}/${MODULE}/bin)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/${BUILD_NODE}/${MODULE}/bin)
    endforeach()   

    #include_directories(${PROJECT_SOURCE_DIR}/kantcpp/servant/protocol/framework)
    include_directories(${PROJECT_SOURCE_DIR}/kantcpp/util/include)
    if(WIN32)
        #include_directories(${PROJECT_SOURCE_DIR}/kantcpp/util/src/epoll_windows)
    endif()

    #include_directories(${servant_SOURCE_DIR}/servant)

    aux_source_directory(. DIR_SRCS)

    add_executable(${MODULE} ${DIR_SRCS})
    # add_dependencies(${MODULE} FRAMEWORK-PROTOCOL)
    # add_dependencies(${MODULE} kant2cpp)

    target_link_libraries(${MODULE} kantutil)

    if(KANT_SSL)
        target_link_libraries(${MODULE} ${LIB_SSL} ${LIB_CRYPTO})
    endif()

    if(KANT_HTTP2)
        target_link_libraries(${MODULE} ${LIB_HTTP2})
    endif()

    if(NOT WIN32)
        link_libraries(pthread z dl)
    endif()
endmacro()

# add_subdirectory(mysql-tool)
# add_subdirectory(ConfigServer)
add_subdirectory(KantNode)
# add_subdirectory(NotifyServer)
# add_subdirectory(PatchServer)
# add_subdirectory(AdminRegistryServer)
# add_subdirectory(RegistryServer)
# add_subdirectory(PropertyServer)
# add_subdirectory(LogServer)
# add_subdirectory(patchclient)
# add_subdirectory(StatServer)
# add_subdirectory(QueryStatServer)
# add_subdirectory(QueryPropertyServer)
############################################################################################
# 打包deploy, 用于部署
