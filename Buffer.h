#pragma once
#include "GapBuffer.h"
#include <cstring>
#include <curses.h>
#include <optional>
#include <string>

class Buffer {
public:
  Buffer() {};
  ~Buffer() {};

  size_t FindLineStart(size_t id) {
    while (m_buf.m_data < m_buf.m_data + id && m_buf[--id] != L'\n') {
    }
    if (m_buf[id] == L'\n') {
      id += 1;
    }
    return id;
  }

  size_t GetLine(size_t id) {
    size_t count = 1;
    while (m_buf.m_data < m_buf.m_data + (id--)) {
      if (m_buf[id] == L'\n')
        count++;
    }
    return count;
  }

  void Draw(WINDOW *win) {
    size_t view_line_start = FindLineStart(m_view_char);
    size_t cursor_line_start = FindLineStart(m_buf.m_front);
    wclear(win);
    wmove(win, 0, 0);
    wprintw(win, "F: %lu T: %lu G: %lu VL: %lu L: %lu", m_buf.m_front,
            m_buf.m_total, m_buf.m_gap, view_line_start, cursor_y);

    wmove(win, 1, 0);

    cchar_t complex_char;
    int cur_y, cur_x, max_y, max_x;
    getmaxyx(win, max_y, max_x);
    for (size_t i = view_line_start; i < m_buf.m_total; i++) {
      if (i == m_buf.m_front) {
        i += m_buf.m_gap;
        if (i >= m_buf.m_total)
          break;
      }

      getyx(win, cur_y, cur_x);

      if (m_buf.m_data[i] == L'\n') {
        wmove(win, cur_y + 1, 0);
        continue;
      }

      wchar_t wch_str[2] = {m_buf.m_data[i], L'\0'};
      if (!m_enable_wrapping) {
        if (cur_x >= max_x - 1) {
          continue;
        }

        if (cur_x >= max_x - 2) {
          wch_str[0] = L'>';
        }
      }

      setcchar(&complex_char, wch_str, WA_NORMAL, 0, NULL);
      wadd_wch(win, &complex_char);
    }

    wmove(win, cursor_y, cursor_x);
  };

  void UpdateCursorData() {
    cursor_x = m_buf.m_front - FindLineStart(m_buf.m_front);
    cursor_y = GetLine(m_buf.m_front);
  }

  void HandleInput(const int res, const wint_t c) {
    if (res == KEY_CODE_YES) {
      switch (c) {
      case KEY_BACKSPACE:
      case KEY_DC:
        m_buf.deleteChar();
        break;
      case KEY_LEFT:
        m_buf.moveBackward();
        break;
      case KEY_RIGHT:
        m_buf.moveForward();
        break;
      }
    } else {
      switch (c) {
      case 127:
      case 8:
        m_buf.deleteChar();
        break;
      default:
        m_buf.insertChar(c);
        break;
      }
    }
    UpdateCursorData();
  };

  GapBuffer m_buf;

private:
  size_t cursor_x = 0;
  size_t cursor_y = 0;
  size_t m_view_char = 0;
  std::optional<std::string> m_path;
  bool m_enable_wrapping = false;
  bool m_virtual;
  bool m_editable;
};
