idf_component_register(SRCS 
                    "retroblue_api.c" "retroblue_backend.c" 
                    "retroblue_settings.c" "rbc_switch_spi.c" 
                    "rbc_switch_reports.c" "rbc_switch_input.c" 
                    "rbc_switch_core.c" "rbc_switch_controller.c" 
                    "rbc_switch_comms.c" "rbc_snes_core.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp32 bt )

