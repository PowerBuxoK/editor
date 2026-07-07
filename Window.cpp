#include "Window.h"
#include "Defines.h"

Window::Window() {};
Window::~Window() {};

void Window::Draw()
{
  if(!m_buf)
    return;

  WINDOW* win;
  win = newwin(m_height, m_width, m_y, m_x);
  m_buf->Draw(win);

  wnoutrefresh(win);
  delwin(win);
};

void Window::HandleInput(const Mode mode, const InputKeypress& kp)
{
  if(!m_buf)
    return;
  m_buf->HandleInput(mode, kp);
};
