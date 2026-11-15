cmake_minimum_required(VERSION 3.22)

project(RoleplayConfiguration LANGUAGES C CXX)

include(FetchContent)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SPSDK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPSDK_INCLUDE_SAMPGDK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    samp_plugin_sdk
    GIT_REPOSITORY https://github.com/guil2k7/samp-plugin-sdk.git
    GIT_TAG c4a32be5a4155924d0b809c78466490869d9349a
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(samp_plugin_sdk)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG 55f93686c01528224f448c19128836e7df245f72
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(roleplay_configuration STATIC
    src/configuration.cpp
)
target_include_directories(roleplay_configuration
    PUBLIC include
)
target_link_libraries(roleplay_configuration
    PUBLIC nlohmann_json::nlohmann_json
)
target_compile_features(roleplay_configuration PUBLIC cxx_std_17)

add_library(roleplay_configuration_plugin MODULE
    src/plugin.cpp
)
target_link_libraries(roleplay_configuration_plugin
    PRIVATE roleplay_configuration samp-plugin-sdk
)
set_target_properties(roleplay_configuration_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "roleplay_config"
)

if(MSVC)
    target_compile_options(roleplay_configuration PRIVATE /W4 /WX)
    target_compile_options(roleplay_configuration_plugin PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(roleplay_configuration PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(roleplay_configuration_plugin PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(WIN32 AND MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_sources(roleplay_configuration_plugin PRIVATE roleplay_config.def)
elseif(MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_options(roleplay_configuration_plugin PRIVATE -Wl,--kill-at)
endif()

if(BUILD_TESTING)
    add_executable(roleplay_configuration_tests
        tests/configuration_tests.cpp
    )
    target_link_libraries(roleplay_configuration_tests PRIVATE roleplay_configuration)
    target_compile_features(roleplay_configuration_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(roleplay_configuration_tests PRIVATE /W4 /WX)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(roleplay_configuration_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    add_test(
        NAME roleplay_configuration_tests
        COMMAND roleplay_configuration_tests "${PROJECT_SOURCE_DIR}/../config"
    )
endif()
