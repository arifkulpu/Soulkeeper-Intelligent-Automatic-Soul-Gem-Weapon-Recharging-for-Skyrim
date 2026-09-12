cmake_minimum_required(VERSION 3.23)

project(
    Soulkeeper
    VERSION 1.0.0
    DESCRIPTION "Smart Weapon Charging System for Skyrim SE, AE and VR"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# 1. Fetch spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# 2. Fetch rapidcsv
FetchContent_Declare(
    rapidcsv
    GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
    GIT_TAG        v8.84
)
FetchContent_MakeAvailable(rapidcsv)
set(RAPIDCSV_INCLUDE_DIRS ${rapidcsv_SOURCE_DIR}/src CACHE PATH "RapidCSV include directory" FORCE)

# 3. Fetch xbyak
FetchContent_Declare(
    xbyak
    GIT_REPOSITORY https://github.com/herumi/xbyak.git
    GIT_TAG        v7.21
)
FetchContent_MakeAvailable(xbyak)

# 4. Fetch simpleini
FetchContent_Declare(
    simpleini
    GIT_REPOSITORY https://github.com/brofield/simpleini.git
    GIT_TAG        v4.22
)
FetchContent_MakeAvailable(simpleini)

# 5. Configure CommonLibSSE-NG options
set(ENABLE_SKYRIM_AE ON CACHE BOOL "Enable Skyrim AE support" FORCE)
set(ENABLE_SKYRIM_SE ON CACHE BOOL "Enable Skyrim SE support" FORCE)
set(ENABLE_SKYRIM_VR ON CACHE BOOL "Enable Skyrim VR support" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Disable CommonLibSSE tests" FORCE)
set(SKSE_SUPPORT_XBYAK ON CACHE BOOL "Enable Xbyak support" FORCE)

add_subdirectory(extern/CommonLibSSE-NG)

# Source and header files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        include
        ${rapidcsv_SOURCE_DIR}/src
        ${spdlog_SOURCE_DIR}/include
        ${xbyak_SOURCE_DIR}
        ${simpleini_SOURCE_DIR}
        extern/CommonLibSSE-NG/include
)

target_include_directories(
    CommonLibSSE
    PUBLIC
        ${xbyak_SOURCE_DIR}
        ${simpleini_SOURCE_DIR}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
        spdlog::spdlog
        xbyak::xbyak
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        SKSE_SUPPORT_XBYAK=1
        NOMINMAX
        WIN32_LEAN_AND_MEAN
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /Zc:preprocessor /utf-8 /EHsc /bigobj)
endif()
