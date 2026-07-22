#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

class GapBuffer
{
public:
  GapBuffer();
  ~GapBuffer();

  void moveCursor(int delta);
  void moveTo(size_t id);
  void grow(size_t size);
  void insertChar(wchar_t c);
  void deleteChar(size_t count = 1);
  void deleteCharFront(size_t count = 1);
  void moveForward();
  void moveBackward();
  void moveUp(size_t cursor_x);
  void moveDown(size_t cursor_x);
  size_t LineLength(size_t id);
  size_t FindLineStart(size_t id);
  size_t GetLine(size_t id);
  void Clean();
  size_t size();
  void SetText(const std::wstring& str);
  std::wstring GetString();

  wchar_t operator[](size_t i)
  {
    if(i < m_front)
    {
      return m_data[i];
    }
    else
    {
      return m_data[i + m_gap];
    }
  }

  size_t m_total  = 0;
  size_t m_front  = 0;
  size_t m_gap    = 0;
  wchar_t* m_data = nullptr;
};
