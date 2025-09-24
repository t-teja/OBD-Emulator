# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(obd2_emulator C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(obd2_emulator
    obd2_emulator.c
    obd2_protocol.c
    obd2_handler.c
    obd2_dtc.c
    vehicle_data.c
    RP2350-CAN-Demo\ \(1\)/C/rp2350_can/xl2515.c
    )

# Set program name and version
pico_set_program_name(obd2_emulator "OBD2_Emulator")
pico_set_program_version(obd2_emulator "1.0")

# Enable/disable output over UART/USB
pico_enable_stdio_uart(obd2_emulator 0)
pico_enable_stdio_usb(obd2_emulator 1)

# pull in common dependencies
target_link_libraries(obd2_emulator
    pico_stdlib
    hardware_spi
    )

# Add include directories
target_include_directories(obd2_emulator PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/RP2350-CAN-Demo\ \(1\)/C/rp2350_can
    )

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(obd2_emulator pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(obd2_emulator)

# add url via pico_set_program_url
