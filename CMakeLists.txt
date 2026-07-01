cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(editor)

find_package(ICU COMPONENTS i18n io uc)
if(WIN32)
	message("Compiling on Windows, using PDCurses")
	find_package(PDCurses)
else()
	message("Compiling not on Windows using NCurses")
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
endif()


add_executable(editor
	./main.cpp
	./Buffer.cpp
	./UTFHelpers.cpp
)

target_compile_definitions(editor PRIVATE NCURSES_WIDECHAR=1)


target_include_directories(editor PRIVATE ./include/)
IF (WIN32)
	target_link_libraries(my_app PRIVATE PDCurses::PDCurses)
ELSE()
	target_link_libraries(editor PRIVATE Curses::Curses)
ENDIF()

target_link_libraries(editor PRIVATE ICU::i18n ICU::io ICU::uc)

