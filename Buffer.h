#pragma once
#include "GapBuffer.h"
#include <curses.h>
#include <optional>
#include <string>

class Buffer {
public:
  Buffer() {};
  ~Buffer() {};

  void Draw(WINDOW *win) {
    wmove(win, 0, 0);
    wprintw(win, "F: %lu T: %lu G: %lu", m_buf.m_front, m_buf.m_total,
            m_buf.m_gap);

    wmove(win, 1, 0);

    cchar_t complex_char;
    int cur_y, cur_x, max_y, max_x;
    getmaxyx(win, max_y, max_x);
    for (size_t i = 0; i < m_buf.m_total; i++) {
      if (i == m_buf.m_front) {
        i += m_buf.m_gap;
        if (i >= m_buf.m_total)
          break;
      }

      getyx(win, cur_y, cur_x);

      wchar_t wch_str[2] = {m_buf.m_data[i], L'\0'};
      if (!m_enable_wrapping) {
        if (cur_x >= max_x - 1) {
          break;
        }

        if (cur_x >= max_x - 2) {
          wch_str[0] = L'>';
        }
      }

      setcchar(&complex_char, wch_str, WA_NORMAL, 0, NULL);
      wadd_wch(win, &complex_char);
    }

    wmove(win, 1, m_buf.m_front);
  };

  void HandleInput(const int res, const wint_t c) {
    if (res == KEY_CODE_YES) {
      switch (c) {
      case KEY_BACKSPACE:
      case KEY_DC:
        m_buf.deleteChar();
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
  };

  GapBuffer m_buf;

private:
  std::optional<std::string> m_path;
  bool m_enable_wrapping = false;
  bool m_virtual;
  bool m_editable;
};
