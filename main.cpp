#include <curses.h>

int main(int argc, char *argv[]) {
  bool quit = false;
  initscr();
  noecho();
  addstr("TEST");
  refresh();
  while (!quit) {
    wchar_t c = getch();
    switch (c) {
    case 'q':
      quit = true;
      break;
    }
  }
  endwin();
  return 0;
}
