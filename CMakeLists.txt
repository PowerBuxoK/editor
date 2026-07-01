cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(editor)

find_package(ICU COMPONENTS i18n io uc REQUIRED)

add_executable(editor
    ./main.cpp
    ./Buffer.cpp
    ./UTFHelpers.cpp
)

target_include_directories(editor PRIVATE ./include/)

if(WIN32)
    find_path(PDCURSES_INCLUDE_DIR 
        NAMES curses.h
        DOC "Path to PDCurses include directory"
    )
    find_library(PDCURSES_LIBRARY 
        NAMES pdcurses
        DOC "Path to PDCurses library"
    )

    if(NOT PDCURSES_INCLUDE_DIR OR NOT PDCURSES_LIBRARY)
        message(FATAL_ERROR "PDCursesMod not found!")
    endif()

    target_include_directories(editor PRIVATE ${PDCURSES_INCLUDE_DIR})
    target_link_libraries(editor PRIVATE ${PDCURSES_LIBRARY})

    target_compile_definitions(editor PRIVATE PDC_WIDE)

else()
    set(CURSES_NEED_NCURSES TRUE)
    set(CURSES_NEED_WIDE TRUE)
    find_package(Curses REQUIRED)

    if(Curses_FOUND AND NOT TARGET Curses::Curses)
      add_library(Curses::Curses INTERFACE IMPORTED)
      set_target_properties(
        Curses::Curses
        PROPERTIES
          INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
          INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
      )
    endif()

    target_link_libraries(editor PRIVATE Curses::Curses)
    
    target_compile_definitions(editor PRIVATE NCURSES_WIDECHAR=1)
endif()

target_link_libraries(editor PRIVATE ICU::i18n ICU::io ICU::uc)
