cmake_minimum_required(VERSION 4.3)
project(c2p2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include modules)

file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
file(GLOB_RECURSE CLI_SOURCES "src/cli/*.cpp")
file(GLOB_RECURSE TUI_SOURCES "src/tui/*.cpp")
file(GLOB_RECURSE MODULE_SOURCES "modules/*.cpp")

include(FetchContent)
set(ENABLE_TEST OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# LibArchive
set(ENABLE_TAR OFF CACHE BOOL "" FORCE)
set(ENABLE_CPIO OFF CACHE BOOL "" FORCE)
set(ENABLE_CAT OFF CACHE BOOL "" FORCE)
set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

# Exiv2
set(EXIV2_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(EXIV2_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(EXIV2_ENABLE_BROTLI OFF CACHE BOOL "" FORCE)
set(EXIV2_ENABLE_WEBP OFF CACHE BOOL "" FORCE)
set(EXIV2_ENABLE_PNG OFF CACHE BOOL "" FORCE)
set(EXIV2_ENABLE_INIH OFF CACHE BOOL "" FORCE)

# dependencies
FetchContent_Declare(ftxui GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI GIT_TAG main)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_Declare(Exiv2 GIT_REPOSITORY https://github.com/Exiv2/exiv2 GIT_TAG v0.28.8)
FetchContent_Declare(LibArchive GIT_REPOSITORY https://github.com/libarchive/libarchive GIT_TAG v3.8.9)

FetchContent_MakeAvailable(ftxui json LibArchive Exiv2)

# executable
add_executable(c2p2
        main.cpp
        ${CORE_SOURCES}
        ${CLI_SOURCES}
        ${TUI_SOURCES}
        ${MODULE_SOURCES}
)

# linker
if(MSVC)
    target_compile_options(c2p2 PRIVATE /Gy)
    target_link_options(c2p2 PRIVATE /OPT:REF)
else()
    target_compile_options(c2p2 PRIVATE -ffunction-sections -fdata-sections)
    if(APPLE)
        target_link_options(c2p2 PRIVATE -Wl,-dead_strip)
    else()
        target_link_options(c2p2 PRIVATE -Wl,--gc-sections)
    endif()
endif()

# LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
    set_property(TARGET c2p2 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Link
target_link_libraries(c2p2 PRIVATE
        archive_static
        exiv2lib
        ftxui::screen
        ftxui::dom
        ftxui::component
        nlohmann_json::nlohmann_json
)