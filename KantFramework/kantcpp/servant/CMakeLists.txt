cmake_minimum_required(VERSION 3.2)

project(servant)

# if(KANT_OPENTRACKING)
# include_directories(${util_SOURCE_DIR}/include ${OPENTRACKING_INC})
# else()
include_directories(${util_SOURCE_DIR}/include)
# endif()

#调用kant2cpp, 生成kant对应的文件
macro(complice_kant OUT_DEPENDS_LIST HEADER INCLUDE)
    set(DEPENDS_LIST)

    FILE(GLOB SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.kant")

    set(CLEAN_LIST)

    #设置kant生成后, cpp文件因为h文件的include路径
    set(HEADER_STRING)
    if (NOT "${HEADER}" STREQUAL "")
        set(HEADER_STRING "--header=${HEADER}")
    endif ()

    #设置kant文件搜索路径
    set(INCLUDE_STRING "")
    set(HEADER_STRING)
    if (NOT "${INCLUDE}" STREQUAL "")
        set(INCLUDE_STRING "--include=${INCLUDE}")
    endif ()

    foreach (FILE ${SRC_LIST})

        get_filename_component(NAME_WE ${FILE} NAME_WE)

        # 生成kant文件
        set(KANT_IN ${FILE})
        set(KANT_H ${NAME_WE}.h)

        add_custom_command(
                OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${KANT_H}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                DEPENDS kant2cpp ${FILE}
                COMMAND ${KANT2CPP} --with-kant --without-trace ${HEADER_STRING} ${INCLUDE_STRING} ${KANT_IN}
                COMMENT "${KANT2CPP} --with-kant --without-trace ${HEADER_STRING} ${INCLUDE_STRING} ${KANT_IN}")

        list(APPEND DEPENDS_LIST ${CMAKE_CURRENT_SOURCE_DIR}/${KANT_H})

        #设置需要清除的文件
        get_filename_component(PATH ${FILE} PATH)
        list(APPEND CLEAN_LIST ${PATH}/${NAME_WE}.h)
        if (EXISTS ${PATH}/${NAME_WE}.cpp)
            list(APPEND CLEAN_LIST ${PATH}/${NAME_WE}.cpp)
        endif (EXISTS ${PATH}/${NAME_WE}.cpp)

    endforeach (FILE)

    set(OUT_DEPENDS_LIST ${DEPENDS_LIST})

    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_LIST}")

endmacro()

add_subdirectory(protocol/servant)
#add_subdirectory(protocol/framework)
add_subdirectory(tup)
add_subdirectory(libservant)

install(DIRECTORY servant DESTINATION include)
install(DIRECTORY promise DESTINATION include)
install(DIRECTORY tup DESTINATION include)
#install(DIRECTORY jmem DESTINATION include)
#install(DIRECTORY script DESTINATION .
#        PATTERN "*.sh"
#        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE)
#install(DIRECTORY makefile DESTINATION .)

