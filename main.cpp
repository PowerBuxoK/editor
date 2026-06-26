#include "GapBuffer.h"
#include <argparser/argparser.cpp>
#include <cwchar>
#include <ncurses.h>
#include <string>

int main(int argc, char *argv[]) {
  setlocale(LC_CTYPE, "C.UTF-8");
  bool quit = false;
  initscr();
  noecho();

  keypad(stdscr, TRUE);

  addstr("TEST");
  move(1, 0);
  refresh();
  GapBuffer buf;
  wint_t c;

  while (!quit) {
    int res = get_wch(&c);
    if (res == ERR)
      break;

    clear();
    move(3, 0);
    printw("Code: %i (Type: %s)", c, (res == KEY_CODE_YES) ? "KEY" : "CHAR");

    if (res == KEY_CODE_YES) {
      switch (c) {
      case KEY_BACKSPACE:
      case KEY_DC:
        buf.deleteChar();
        break;
      case KEY_LEFT:
        buf.moveBackward();
        break;
      case KEY_RIGHT:
        buf.moveForward();
        break;
      }
    } else {
      switch (c) {
      case 'q':
        quit = true;
        break;
      case 127:

      case 8:
        buf.deleteChar();
        break;
      default:
        buf.insertChar(c);
        break;
      }
    }

    move(0, 0);
    printw("F: %lu T: %lu G: %lu", buf.m_front, buf.m_total, buf.m_gap);

    move(1, 0);

    cchar_t complex_char;
    for (size_t i = 0; i < buf.m_total; i++) {
      if (i == buf.m_front) {
        i += buf.m_gap;
        if (i >= buf.m_total)
          break;
      }

      wchar_t wch_str[2] = {buf.m_data[i], L'\0'};
      setcchar(&complex_char, wch_str, WA_NORMAL, 0, NULL);
      add_wch(&complex_char);
    }

    move(1, buf.m_front);

    refresh();
  }
  endwin();
  return 0;
}
