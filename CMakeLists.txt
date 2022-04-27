# Standard routine
cmake_minimum_required(VERSION 3.16)
project(ts_ka4alka)

# Use AUI.Boot
file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

# import AUI
auib_import(AUI https://github.com/aui-framework/aui
        VERSION f2450ce
        COMPONENTS core views curl crypt)


# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(ts_ka4alka)

find_package(OpenSSL REQUIRED)

# Link required libs
aui_link(ts_ka4alka PRIVATE aui::core aui::views aui::curl OpenSSL::Crypto)

