# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work == 
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
    set(sdkVersion 2.2.0)
    set(toolchainVersion 14_2_Rel1)
    set(picotoolVersion 2.2.0-a4)
    set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==


cmake_minimum_required(VERSION 3.27.4)

# Select board:
#set(PICO_BOARD pico2_w CACHE STRING "Board type")
set(PICO_BOARD pico CACHE STRING "Board type")


include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(CMAKE_C_FLAGS_RELEASE "-O3")

# --- SOURCE FILES ---
add_executable(${PROJECT_NAME}
    main.c
    DAC/DAC.c
    cli/cli.c
)

# --- INCLUDE DIRECTORIES ---
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    DAC
    cli
)

# --- LINK LIBRARIES ---
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_clocks
    hardware_spi
)


# --- BUILD OUTPUTS ---
pico_add_extra_outputs(${PROJECT_NAME})

# --- USB DEVICE DESCRIPTORS ---
target_compile_definitions(${PROJECT_NAME} PRIVATE
    USBD_VID=0x2E8A
    USBD_PID=0x000A
    USBD_MANUFACTURER="CFD Lab"
    USBD_PRODUCT="CFD DAC8554 Controller"
)

# --- STDIO SETTINGS ---
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# --- BUILD OUTPUTS ---
pico_add_extra_outputs(${PROJECT_NAME})