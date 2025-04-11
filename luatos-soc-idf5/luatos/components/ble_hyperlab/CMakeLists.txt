idf_component_register(
    SRCS 
        "src/ble_controller.cpp"
        "src/ble_handlers.cpp"
        ${LUATOS_ROOT}/luat/modules
        ${LUATOS_ROOT}/lua/src
        ${LUATOS_ROOT}/components/printf
    INCLUDE_DIRS 
        "src"
        ../../include
        ${LUATOS_ROOT}/lua/include
        ${LUATOS_ROOT}/luat/include
        ${LUATOS_ROOT}/components/printf
        # ${CMAKE_CURRENT_SOURCE_DIR}/../arduino/libraries/ArduinoJson/src
    REQUIRES app_update esp-nimble arduino
)