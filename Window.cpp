#include "Window.h"

Window::Window() {};
Window::~Window() {};

void Window::Draw()
{
  if (!m_buf)
    return;

  WINDOW *win;
  win = newwin(m_height, m_width, m_y, m_x);
  m_buf->Draw(win);

  wrefresh(win);
  delwin(win);
};

void Window::HandleInput(const Mode mode, const int res, const wint_t c)
{
  if (!m_buf)
    return;
  m_buf->HandleInput(mode, res, c);
};
