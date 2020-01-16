set(COMPONENT_SRCS "main.c"
                    "../components/joystick/src/joystick.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "."
                            "../components/joystick"
)

register_component()
