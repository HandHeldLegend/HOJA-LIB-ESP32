#set(COMPONENT_SRCS "rbc_switch_core.c" "main.c")
#set(COMPONENT_ADD_INCLUDEDIRS "")

#register_component()

idf_component_register(SRCS "main.c" "retroblue_api.c" "retroblue_backend.c" "retroblue_settings.c"
"rbc_switch_core.c" "rbc_switch_reports.c" "rbc_switch_comms.c" "rbc_switch_controller.c" "rbc_switch_input.c" "rbc_switch_spi.c"
                    INCLUDE_DIRS ".")
