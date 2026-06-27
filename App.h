#pragma once

#include "Buffer.h"
#include "Manager.h"
#include "Window.h"
#include <cstdint>
class App {
public:
  App() : m_manager(m_windows) {
    Window &v = m_windows.emplace_back();
    v.layer = 0;
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    v.m_x = 0;
    v.m_y = 0;
    v.m_width = max_x;
    v.m_height = max_y;
    Buffer &buf = m_buffers.emplace_back();
    v.m_buf = &buf;
  };
  ~App() {};

  void run() {
    while (!m_stop) {
      Draw();
      HandleInput();
    }
  };
  void HandleInput() {
    wint_t c;
    int res = get_wch(&c);
    if (res == ERR)
      return;
    HandleKeypress(res, c);
  };
  void Draw() { m_manager.Draw(); };
  void HandleKeypress(const int res, const wint_t c) {
    if (m_windows.size() <= m_focus)
      return;
    m_windows.at(m_focus).HandleInput(res, c);
  };

private:
  uint64_t m_focus = 0;
  bool m_stop = false;
  std::vector<Buffer> m_buffers;
  std::vector<Window> m_windows;
  Manager m_manager;
};
