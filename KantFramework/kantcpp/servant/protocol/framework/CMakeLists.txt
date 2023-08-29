set(OUT_DEPENDS_LIST)

set(SERVANT_PROTOCOL "${servant_SOURCE_DIR}/protocol/servant")

complice_kant(OUT_DEPENDS_LIST "" ${SERVANT_PROTOCOL})

add_custom_target(FRAMEWORK-PROTOCOL ALL DEPENDS ${OUT_DEPENDS_LIST} COPY-SERVENT-KANT)

set(FRAMEWORKPROTOCOLS
        AdminReg.kant
        AdminReg.h
        MonitorQuery.kant
        MonitorQuery.h
        Node.kant
        Node.h
        NodeDescriptor.kant
        NodeDescriptor.h
        Patch.kant
        Patch.h
        Registry.kant
        Registry.h
        RegistryDescriptor.kant
        RegistryDescriptor.h)

INSTALL(FILES ${FRAMEWORKPROTOCOLS} DESTINATION  include/framework)

