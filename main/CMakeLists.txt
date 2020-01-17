set(COMPONENT_SRCS "main.c"
                    "../components/controller/src/controller.c"
                    "../components/mw_proto/src/mw_proto.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "."
                            "../components/controller"
                            "../components/mw_proto"
)

register_component()
