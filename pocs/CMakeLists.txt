cmake_minimum_required(VERSION 3.20)
project(tmux-pocs LANGUAGES C)

# Windows 10+ target
set(CMAKE_SYSTEM_VERSION 10.0)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# Common compile definitions for Windows 10+
add_compile_definitions(_WIN32_WINNT=0x0A00)
add_compile_definitions(WIN32_LEAN_AND_MEAN)

# Warning level
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# POC 01: ConPTY - Core PTY functionality
add_subdirectory(01-conpty)

# POC 02: Named Pipes - IPC replacement
add_subdirectory(02-named-pipes)

# POC 03: Process Management
add_subdirectory(03-process)

# POC 04: Console Events - Signal replacement
add_subdirectory(04-console-events)

# POC 05: PDCurses - ncurses replacement (optional, requires pdcurses)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/05-pdcurses/curses.h" OR DEFINED PDCURSES_INCLUDE_DIR)
    add_subdirectory(05-pdcurses)
else()
    message(STATUS "Skipping 05-pdcurses: PDCurses not found. Run setup_pdcurses.ps1 first.")
endif()

# POC 06: libevent on Windows (optional, requires libevent)
find_package(Libevent CONFIG QUIET)
if(Libevent_FOUND)
    add_subdirectory(06-libevent-win)
else()
    message(STATUS "Skipping 06-libevent-win: libevent not found. Install via vcpkg.")
endif()

message(STATUS "")
message(STATUS "=== tmux-pocs Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")
