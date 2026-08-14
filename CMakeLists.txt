cmake_minimum_required(VERSION 3.16)
project(snes_emulator LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB COMPONENT_SOURCES CONFIGURE_DEPENDS
    src/components/*.cpp
)
file(GLOB MISC_SOURCES CONFIGURE_DEPENDS
    src/misc/*.cpp
)
file(GLOB CPU_OPERATIONS_SOURCES CONFIGURE_DEPENDS
    src/cpu_operations/*.cpp
)

file(GLOB TESTING_SOURCES CONFIGURE_DEPENDS
    src/testing/*.cpp
)
set(SNES_SOURCES
    ${COMPONENT_SOURCES}
    ${MISC_SOURCES}
    ${CPU_OPERATIONS_SOURCES}
    ${MAPPER_SOURCES}
    ${TESTING_SOURCES}
    src/main.cpp
)

find_package(SDL3 REQUIRED CONFIG)
add_executable(snes_emulator ${SNES_SOURCES})

option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if(ENABLE_SANITIZERS)
    target_compile_options(snes_emulator PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g
    )

    target_link_options(snes_emulator PRIVATE
        -fsanitize=address,undefined
    )
endif()

target_include_directories(snes_emulator PRIVATE
    ${CMAKE_SOURCE_DIR}/include/stores
    ${CMAKE_SOURCE_DIR}/include/components
    ${CMAKE_SOURCE_DIR}/include/coprocessors
    ${CMAKE_SOURCE_DIR}/include/misc
    ${CMAKE_SOURCE_DIR}/include/cpu_operations
    ${CMAKE_SOURCE_DIR}/include/mappers
    ${CMAKE_SOURCE_DIR}/include/testing
)

target_link_libraries(snes_emulator PRIVATE SDL3::SDL3)

set(SNES_OPT_FLAGS
    -O3
    -march=native
    -mtune=native
    -flto=auto          
    -funroll-loops
    -fomit-frame-pointer
    -fstrict-aliasing
    -fno-semantic-interposition
    -fno-rtti
    -fmerge-all-constants
    -fno-plt
)

target_compile_options(snes_emulator PRIVATE
    $<$<CONFIG:Release>:${SNES_OPT_FLAGS};-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:${SNES_OPT_FLAGS};-g>
)

target_link_options(snes_emulator PRIVATE
    $<$<CONFIG:Release>:-O3;-flto=auto>
    $<$<CONFIG:RelWithDebInfo>:-O3;-flto=auto>
)
