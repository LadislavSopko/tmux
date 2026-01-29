# POC 05: PDCurses - ncurses Replacement
# Validates PDCurses as drop-in replacement for ncurses

# Look for PDCurses in local directory or specified path
if(DEFINED PDCURSES_INCLUDE_DIR)
    set(PDCURSES_INCLUDE ${PDCURSES_INCLUDE_DIR})
    set(PDCURSES_LIB_DIR ${PDCURSES_LIBRARY_DIR})
else()
    set(PDCURSES_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})
    set(PDCURSES_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_executable(curses_poc curses_poc.c)

target_include_directories(curses_poc PRIVATE ${PDCURSES_INCLUDE})
target_link_directories(curses_poc PRIVATE ${PDCURSES_LIB_DIR})
target_link_libraries(curses_poc PRIVATE pdcurses user32 advapi32)
