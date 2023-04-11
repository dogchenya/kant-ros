
##########################################################################################
#生成kant文件, 并将.h文件copy到servant目录下

set(OUT_DEPENDS_LIST)

#在当前目录生成kant对应的文件
complice_kant(OUT_DEPENDS_LIST "servant" "")

FILE(GLOB SRC_LIST "*.kant")

set(CLEAN_LIST)

#将生成kant文件copy到对应的源码目录
foreach (FILE ${SRC_LIST})

    #copy .h文件
    get_filename_component(NAME_WE ${FILE} NAME_WE)
    set(KANT_H ${NAME_WE}.h)
    set(CP_SOURCE_H_DIR "${PROJECT_SOURCE_DIR}/servant")

    add_custom_command(
            OUTPUT ${CP_SOURCE_H_DIR}/${KANT_H}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy ${KANT_H} ${CP_SOURCE_H_DIR}
            DEPENDS ${FILE}
            COMMENT "${CMAKE_COMMAND} -E copy ${KANT_H} ${CP_SOURCE_H_DIR}")

    #设置依赖
    list(APPEND OUT_DEPENDS_LIST ${CP_SOURCE_H_DIR}/${KANT_H})

    #设置需要清除的.h文件
    list(APPEND CLEAN_LIST ${CP_SOURCE_CPP_DIR}/${NAME_WE}.h)

endforeach ()

#设置需要清除的文件
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_LIST}")

add_custom_target(COPY-SERVENT-KANT ALL DEPENDS TUP-KANT ${OUT_DEPENDS_LIST})

set(KANTPROTOCOLS
AdminF.kant
AdminF.h
AuthF.kant
AuthF.h
BaseF.kant
BaseF.h
ConfigF.kant
ConfigF.h
EndpointF.kant
EndpointF.h
LogF.kant
LogF.h
NodeF.kant
NodeF.h
NotifyF.kant
NotifyF.h
PropertyF.kant
PropertyF.h
QueryF.kant
QueryF.h
StatF.kant
StatF.h)

install(DIRECTORY . DESTINATION include/servant
        PATTERN "*.kant"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE)

if(WIN32)
INSTALL(FILES ${KANTPROTOCOLS} DESTINATION   c:\\kantproto\\protocol\\)
elseif(APPLE)
INSTALL(FILES ${KANTPROTOCOLS} DESTINATION   $ENV{HOME}/kantproto/protocol/)
elseif(UNIX)
INSTALL(FILES ${KANTPROTOCOLS} DESTINATION   /home/kantproto/protocol/)
endif()
