#pragma once
#include "Defines.h"
#include "GapBuffer.h"
#include <cstring>
#include <curses.h>
#include <optional>
#include <string>

class App;

class Buffer {
public:
  Buffer(App &app) : m_buf(), m_app(app) {};
  ~Buffer() {};

  void Draw(WINDOW *win) {
    size_t view_line_start =
        m_buf.FindLineStart(std::min(m_buf.m_front, m_view_char));
    size_t cursor_line_start = m_buf.FindLineStart(m_buf.m_front);
    wclear(win);
    wmove(win, 0, 0);

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

    wmove(win, cursor_y - m_buf.GetLine(view_line_start) + 1, cursor_x);
    cursor_x_vis = cursor_x;
    cursor_y_vis = cursor_y - m_buf.GetLine(view_line_start);
  };

  void UpdateCursorData() {
    cursor_x = m_buf.m_front - m_buf.FindLineStart(m_buf.m_front);
    cursor_y = m_buf.GetLine(m_buf.m_front);
  }

  void HandleInput(const Mode mode, const int res, const wint_t c) {
    switch (mode) {
    case Mode::insert:
      if (m_editable)
        HandleInputInsert(res, c);
      break;
    default:
      break;
    }
  };

  bool HandleMacro(const size_t quantifier, const std::wstring &macro);

  void HandleInputInsert(const int res, const wint_t c);

  size_t getCursorX() const { return cursor_x; }
  size_t getCursorY() const { return cursor_y; }

  size_t visualCursorX() const { return cursor_x_vis; }
  size_t visualCursorY() const { return cursor_y_vis; }

  GapBuffer m_buf;

private:
  size_t cursor_x = 0;
  size_t cursor_y = 0;
  size_t cursor_x_vis = 0;
  size_t cursor_y_vis = 0;
  size_t m_view_char = 0;
  std::optional<std::string> m_path;
  App &m_app;
  bool m_enable_wrapping = false;
  bool m_editable = true;
};
