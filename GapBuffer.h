#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

class GapBuffer {
public:
  GapBuffer() : m_total(64), m_front(0), m_gap(64) {
    m_data = (wchar_t *)malloc(m_total * sizeof(wchar_t));
    if (!m_data) {
      std::cout << "Failed to create gap buffer!" << std::endl;
      exit(-1);
    }
  }

  ~GapBuffer() {
    if (m_data)
      free(m_data);
  }

  void moveCursor(int delta) {
    if (delta == 0)
      return;

    size_t len;
    wchar_t *dst, *src;

    if (delta < 0) {
      len = -delta;
      if (len > m_front)
        len = m_front;
      if (len == 0)
        return;
      dst = m_data + m_front + m_gap - len;
      src = m_data + m_front - len;
      m_front -= len;
    } else {
      size_t back = m_total - m_front - m_gap;
      len = delta;
      if (len > back)
        len = back;
      if (len == 0)
        return;
      dst = m_data + m_front;
      src = m_data + m_front + m_gap;
      m_front += len;
    }
    memmove(dst, src, len * sizeof(wchar_t));
  }

  void moveForward() { moveCursor(1); }
  void moveBackward() { moveCursor(-1); }

  void insertChar(const wchar_t ch) {
    if (m_gap == 0) {
      grow(m_total * 2);
    }

    m_data[m_front] = ch;
    m_front++;
    m_gap--;
  }

  void deleteChar() {
    if (m_front > 0) {
      m_front--;
      m_gap++;
    }
  }

  void debugPrint() const {
    std::wcout << L"Text: [";
    for (size_t i = 0; i < m_front; ++i)
      std::wcout << m_data[i];
    for (size_t i = 0; i < m_gap; ++i)
      std::wcout << L"_";
    for (size_t i = m_front + m_gap; i < m_total; ++i)
      std::wcout << m_data[i];
    std::wcout << L"] (Front: " << m_front << L", Gap: " << m_gap << L")"
               << std::endl;
  }

  wchar_t operator[](int i) {
    if (i < m_front) {
      return m_data[i];
    } else {
      return m_data[i + m_gap];
    }
  }

  size_t m_total = 0;
  size_t m_front = 0;
  size_t m_gap = 0;
  wchar_t *m_data = nullptr;

private:
  void grow(size_t new_capacity) {
    if (new_capacity <= m_total)
      return;

    size_t old_total = m_total;
    size_t back_len = old_total - m_front - m_gap;

    wchar_t *new_data =
        (wchar_t *)realloc(m_data, new_capacity * sizeof(wchar_t));
    if (!new_data) {
      std::cout << "Grow failed!" << std::endl;
      exit(-1);
    }
    m_data = new_data;
    m_total = new_capacity;

    size_t new_gap = m_total - m_front - back_len;
    if (back_len > 0) {
      memmove(m_data + m_front + new_gap, m_data + m_front + m_gap,
              back_len * sizeof(wchar_t));
    }
    m_gap = new_gap;
  }
};
