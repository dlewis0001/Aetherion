cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(INJECTION_PIO_PATH ${CMAKE_CURRENT_LIST_DIR}/src/injection.pio)
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)


# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================


set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(injection_pio C CXX ASM)
pico_sdk_init()

add_executable(Aetherion-v1.0 
main.c
src/descriptors.c
src/injection.c
src/ostrich.c
src/mutexes.c
src/abstract_layer.c
src/flash_memory.c
src/developer_reset.c
src/developer_tools.c
)

pico_generate_pio_header(Aetherion-v1.0 ${CMAKE_CURRENT_LIST_DIR}/src/injection.pio)
pico_add_extra_outputs(Aetherion-v1.0)

target_include_directories(Aetherion-v1.0 PRIVATE 
${PICO_SDK_PATH}/lib/tinyusb/src)

target_link_libraries(Aetherion-v1.0
pico_stdlib
hardware_pio
hardware_gpio
hardware_sync
pico_multicore
tinyusb_device
tinyusb_board
tinyusb_host
hardware_flash
)

# pico_set_binary_type(Aetherion-v1.0 no_flash) # if used everything will run in RAM space (only use when developing or nothing will save)
