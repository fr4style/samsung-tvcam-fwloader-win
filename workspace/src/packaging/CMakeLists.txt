cmake_minimum_required(VERSION 3.16)

project(samsung_fwloader LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(SRC_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

add_executable(samsung-fwloader
    "${SRC_ROOT}/cli_interface/main.c"
    "${SRC_ROOT}/state_machine/fwloader.c"
    "${SRC_ROOT}/usb_layer/usb_win.c"
    "${SRC_ROOT}/firmware_parser/firmware_parser.c"
)

target_include_directories(samsung-fwloader PRIVATE
    "${PROJECT_ROOT}/include"
    "${SRC_ROOT}/cli_interface"
    "${SRC_ROOT}/state_machine"
    "${SRC_ROOT}/usb_layer"
    "${SRC_ROOT}/firmware_parser"
)

target_link_directories(samsung-fwloader PRIVATE "${PROJECT_ROOT}/lib")
target_link_libraries(samsung-fwloader PRIVATE usb-1.0 setupapi)
target_link_options(samsung-fwloader PRIVATE -static-libgcc)
