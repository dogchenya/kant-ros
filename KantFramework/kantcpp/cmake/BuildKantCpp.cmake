
macro(build_kant_server MODULE DEPS)

project(${MODULE})

include_directories(./)

FILE(GLOB_RECURSE DIR_SRCS "*.cpp")

FILE(GLOB_RECURSE KANT_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.kant")
FILE(GLOB_RECURSE PB_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

set(KANT_LIST_DEPENDS)
set(PB_LIST_DEPENDS)
if (KANT_LIST)
    set(CLEAN_LIST)

    foreach (KANT_SRC ${KANT_LIST})
        get_filename_component(NAME_WE ${KANT_SRC} NAME_WE)
        get_filename_component(PATH ${KANT_SRC} PATH)

        set(KANT_H ${NAME_WE}.h)

        set(CUR_KANT_GEN ${PATH}/${KANT_H})
        LIST(APPEND KANT_LIST_DEPENDS ${CUR_KANT_GEN})
        
        add_custom_command(OUTPUT ${CUR_KANT_GEN}
                WORKING_DIRECTORY ${PATH}
                DEPENDS ${KANT2CPP} ${KANT_SRC}
                COMMAND ${KANT2CPP} ${KANT_SRC}
                COMMENT "${KANT2CPP} ${KANT_SRC}")

        list(APPEND CLEAN_LIST ${PATH}/${KANT_H})

    endforeach ()

    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_LIST}")

    set(KANT_TARGET "KANT_${MODULE}")  
    add_custom_target(${KANT_TARGET} ALL DEPENDS ${KANT_LIST_DEPENDS} kant2cpp)

    add_executable(${MODULE} ${DIR_SRCS})

    add_dependencies(${MODULE} ${KANT_TARGET})
    
elseif(PB_LIST)
    set(CLEAN_LIST)
    set(_PROTOBUF_PROTOC ${CMAKE_BINARY_DIR}/src/protobuf/bin/protoc)

    foreach (PB_SRC ${PB_LIST})
        get_filename_component(NAME_WE ${PB_SRC} NAME_WE)
        get_filename_component(PATH ${PB_SRC} PATH)

        set(PB_H ${NAME_WE}.pb.h)
        set(PB_CC ${NAME_WE}.pb.cc)

        set(CUR_PB_GEN ${PATH}/${PB_H} ${PATH}/${PB_CC})
        LIST(APPEND PB_LIST_DEPENDS ${CUR_PB_GEN})

        add_custom_command(OUTPUT ${CUR_PB_GEN}
                WORKING_DIRECTORY ${PATH}
                DEPENDS ${PROTO2KANT} ${_PROTOBUF_PROTOC}
                COMMAND ${_PROTOBUF_PROTOC} -I "${PATH}"
                            "${PB_SRC}" --cpp_out "${PATH}"
                COMMENT "${_PROTOBUF_PROTOC} ${PB_SRC} ${PATH} ${CUR_PB_GEN}")

        list(APPEND CLEAN_LIST ${PATH}/${PB_H} ${PATH}/${PB_CC})
    endforeach ()

    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_LIST}")

    set(KANT_TARGET "KANT_${MODULE}")  
    add_custom_target(${KANT_TARGET} ALL DEPENDS ${PB_LIST_DEPENDS})
    add_executable(${MODULE} ${CLEAN_LIST} ${DIR_SRCS})
    add_dependencies(${MODULE} ${KANT_TARGET})
else()
    add_executable(${MODULE} ${DIR_SRCS})
endif()

if("${DEPS}" STREQUAL "")
    add_dependencies(${MODULE} kantservant kantutil)
else()
    string(REPLACE " " ";" DEP_LIST ${DEPS})
    add_dependencies(${MODULE} ${DEP_LIST} kantservant kantutil)
endif()

target_link_libraries(${MODULE} kantservant kantutil)

if(KANT_SSL)
    target_link_libraries(${MODULE} kantservant kantutil ${LIB_SSL} ${LIB_CRYPTO})

    if(WIN32)
        target_link_libraries(${MODULE} Crypt32)
    endif()
endif()

if(KANT_HTTP2)
    target_link_libraries(${MODULE} ${LIB_HTTP2} ${LIB_PROTOBUF})
endif()

if(KANT_GPERF)
    target_link_libraries(${MODULE} ${LIB_GPERF})
endif(KANT_GPERF)

SET(MODULE-TGZ "${CMAKE_BINARY_DIR}/${MODULE}.tgz")
SET(RUN_DEPLOY_COMMAND_FILE "${PROJECT_BINARY_DIR}/run-deploy-${MODULE}.cmake")
FILE(WRITE ${RUN_DEPLOY_COMMAND_FILE} "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/deploy/${MODULE})\n")
IF(WIN32)
    # message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/${MODULE}.exe)
    FILE(APPEND ${RUN_DEPLOY_COMMAND_FILE} "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/${MODULE}.exe ${PROJECT_BINARY_DIR}/deploy/${MODULE}/)\n")
    FILE(APPEND ${RUN_DEPLOY_COMMAND_FILE} "EXECUTE_PROCESS(WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/deploy/ \n COMMAND ${CMAKE_COMMAND} -E tar czfv ${MODULE-TGZ} ${MODULE})\n")
ELSE()
    FILE(APPEND ${RUN_DEPLOY_COMMAND_FILE} "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MODULE} ${PROJECT_BINARY_DIR}/deploy/${MODULE}/)\n")
    FILE(APPEND ${RUN_DEPLOY_COMMAND_FILE} "EXECUTE_PROCESS(WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/deploy/ \n COMMAND ${CMAKE_COMMAND} -E tar czfv ${MODULE-TGZ} ${MODULE})\n")
ENDIF()

#执行命令
add_custom_command(OUTPUT ${MODULE-TGZ}
        DEPENDS ${MODULE}
        COMMAND ${CMAKE_COMMAND} -P ${RUN_DEPLOY_COMMAND_FILE}
        COMMENT "call ${RUN_DEPLOY_COMMAND_FILE}")

add_custom_target(${MODULE}-tar DEPENDS ${MODULE-TGZ})
            
endmacro()

#-----------------------------------------------------------------------
