# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# change it!!
set(LUATOS_ROOT "D:/ble_luatos/LuatOS")

set(EXTRA_COMPONENT_DIRS
             ./components/luat
    )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_component_optional_requires(PRIVATE esp_psram)
project(luatos)


idf_build_set_property(COMPILE_OPTIONS "-D__LUATOS__=1" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DLUAT_FREERTOS_FULL_INCLUDE=1" APPEND)

idf_build_get_property(config_json SDKCONFIG_JSON)

add_custom_command(
    TARGET app
    POST_BUILD
    COMMAND ${python} "${CMAKE_CURRENT_LIST_DIR}/makesoc.py" "${config_json}"
    )

