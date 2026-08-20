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
FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
        GIT_TAG main
)
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(json)

add_executable(c2p2
        main.cpp
        ${CORE_SOURCES}
        ${CLI_SOURCES}
        ${TUI_SOURCES}
        ${MODULE_SOURCES}
)

target_link_libraries(c2p2 PRIVATE ftxui::screen ftxui::dom ftxui::component)
target_link_libraries(c2p2 PRIVATE nlohmann_json::nlohmann_json)