set(COMPONENT_SRCS "main.c"
                    "../components/joystick/src/joystick.c"
                    "../components/mw_proto/src/mw_proto.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "."
                            "../components/joystick"
                            "../components/mw_proto"
)

register_component()
