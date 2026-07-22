cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(editor)

find_package(ICU COMPONENTS i18n io uc)
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses)

if(Curses_FOUND AND NOT TARGET Curses::Curses)
  add_library(Curses::Curses INTERFACE IMPORTED)
  set_target_properties(
    Curses::Curses
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
  )
endif()

add_executable(editor
	./main.cpp
	./Buffer.cpp
	./UTFHelpers.cpp
  ./GapBuffer.cpp
  ./App.cpp
  ./Manager.cpp
  ./Window.cpp
  ./Buffer.cpp
	./CommandManager.cpp
)

target_compile_definitions(editor PRIVATE NCURSES_WIDECHAR=1)

if(IS_DIRECTORY "${CURSES_INCLUDE_DIRS}/ncurses")
	target_include_directories(editor PRIVATE ./include/ ${CURSES_INCLUDE_DIRS}/ncurses)
else()
	target_include_directories(editor PRIVATE ./include/)
endif()

target_link_libraries(editor PRIVATE ICU::i18n ICU::io ICU::uc)
target_link_libraries(editor PRIVATE Curses::Curses)

