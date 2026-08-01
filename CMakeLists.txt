cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(blink C CXX ASM)

include_directories(inc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(blink
    main.c

    src/app.c
    src/buttons.c
    src/display.c
    src/font5x7.c
    src/ui.c
    src/usb_descriptors.c
    src/usb_hid.c
)

target_link_libraries(blink
    pico_stdlib
    hardware_i2c
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(blink 0)
pico_enable_stdio_uart(blink 0)

pico_add_extra_outputs(blink)