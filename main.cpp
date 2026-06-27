#include "App.h"
#include <argparser/argparser.cpp>
#include <ncurses.h>

int main(int argc, char *argv[]) {

  arp::Argparser ap(argc, argv);
  ap.parse();

  setlocale(LC_CTYPE, "C.UTF-8");
  bool quit = false;
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  refresh();

  App app;
  app.run();

  refresh();
  endwin();
  return 0;
}
