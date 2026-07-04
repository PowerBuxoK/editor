#pragma once
#include "Buffer.h"

class Window
{

public:
  unsigned int m_x, m_y;
  unsigned int m_width, m_height;
  std::wstring name = L"Unnamed";

  Buffer *m_buf = nullptr;
  int layer = 0;

  Window();
  ~Window();
  void Draw();
  void HandleInput(const Mode mode, const int res, const wint_t c);
};