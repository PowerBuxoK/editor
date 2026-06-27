#pragma once
#include "Window.h"
#include <algorithm>
#include <ncurses.h>
#include <vector>

class Manager {
public:
  Manager(std::vector<Window> &windows) : m_windows(windows) {};
  ~Manager() {};

  void Draw() {
    std::vector<Window *> windows_sorted;
    windows_sorted.reserve(m_windows.size());

    for (int i = 0; i < m_windows.size(); i++) {
      windows_sorted.emplace_back(&m_windows[i]);
    }

    for (int i = 0; i < m_windows.size(); i++) {
      for (int j = 0; j < m_windows.size(); j++) {
        if (windows_sorted[i]->layer > windows_sorted[j]->layer) {
          std::swap(windows_sorted[i], windows_sorted[j]);
        }
      }
    }

    for (auto &v : windows_sorted) {
      v->Draw();
    }
  };

private:
  std::vector<Window> &m_windows;
};
