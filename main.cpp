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
  std::wstring h;
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
      if (!h.empty())
        h.pop_back();
      break;
    default:
      h += c;
      break;
    }
    clear();
    mvaddwstr(1, 0, h.c_str());
    move(2, 0);
    printw("%i", c);
    refresh();
  }
  endwin();
  return 0;
}
