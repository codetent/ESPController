set(COMPONENT_SRCS "main.c"
                    "../components/controller/src/controller.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "."
                            "../components/controller"
)

register_component()
