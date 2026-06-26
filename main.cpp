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
  addstr("TEST");
  move(1, 0);
  refresh();
  GapBuffer buf;
  wint_t c;
  while (!quit) {
    if (get_wch(&c) != OK)
      break;
    switch (c) {
    case 'q':
      quit = true;
      break;
    case 127:
    case KEY_BACKSPACE:
    case KEY_DC:
      buf.deleteChar();
      break;
    default:
      buf.insertChar(c);
      break;
    }
    clear();
    move(0, 0);
    printw("F: %lu T: %lu G: %lu", buf.m_front, buf.m_total, buf.m_gap);
    move(1, 0);
    // buf.debugPrint();
    cchar_t complex_char;
    for (size_t i = 0; i < buf.m_total; i++) {
      if (i == buf.m_front) {
        i += buf.m_gap;
        continue;
      }
      setcchar(&complex_char, &buf.m_data[i], WA_NORMAL, 0, NULL);
      add_wch(&complex_char);
    }
    refresh();
  }
  endwin();
  return 0;
}
