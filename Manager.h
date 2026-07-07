#pragma once
#include "Window.h"
#include <algorithm>
#include <deque>
#include <ncurses.h>
#include <vector>

class Manager
{
public:
  Manager(std::deque<Window>& windows);
  ~Manager();

  void Draw();

private:
  std::deque<Window>& m_windows;
};
