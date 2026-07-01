#pragma once
#include "Buffer.h"

class Window {

public:
  unsigned int m_x, m_y;
  unsigned int m_width, m_height;
  std::wstring name = L"Unnamed";

  Buffer *m_buf = nullptr;
  int layer = 0;

  Window() {};
  ~Window() {};

  void Draw() {
    if (!m_buf)
      return;

    WINDOW *win;
    win = newwin(m_height, m_width, m_y, m_x);
    m_buf->Draw(win);

    wrefresh(win);
    delwin(win);
  };

  void HandleInput(const Mode mode, const int res, const wint_t c) {
    if (!m_buf)
      return;
    m_buf->HandleInput(mode, res, c);
  };
};
