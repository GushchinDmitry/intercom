set(COMPONENT_SRCS "icom_led.c")
set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
#set(COMPONENT_PRIV_REQUIRES  esp_peripherals audio_sal audio_hal esp_dispatcher display_service)
set(COMPONENT_PRIV_REQUIRES  esp_peripherals display_service)

register_component()