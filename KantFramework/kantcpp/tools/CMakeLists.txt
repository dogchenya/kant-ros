cmake_minimum_required(VERSION 3.2)
project(tools)

include_directories(${util_SOURCE_DIR}/include)

link_libraries(kantutil)

function(kant2language TARGET)
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    endforeach()

    include_directories(${PROJECT_SOURCE_DIR}/kantparse)

    aux_source_directory(. DIR_SRCS)

    link_libraries(kantparse)

    add_executable(${TARGET} ${DIR_SRCS})

    install(TARGETS ${TARGET}
            RUNTIME DESTINATION tools)
endfunction()

IF(UNIX)
add_subdirectory(kantgrammar)
ENDIF(UNIX)

add_subdirectory(kantparse)
#add_subdirectory(kantmerge)
add_subdirectory(kant2cpp)

IF(KANT_PROTOBUF)
#    add_subdirectory(pb2kantcpp)
ENDIF()
