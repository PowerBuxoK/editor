#pragma once
#include "Defines.h"
#include "GapBuffer.h"
#include "UTFHelpers.h"
#include <cstdio>
#include <cstring>
#include <curses.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <vector>

class App;

class Buffer {
public:
  Buffer(App &app) : m_buf(), m_app(app) {};
  ~Buffer() {};

  void Draw(WINDOW *win) {
    int cur_y, cur_x, max_y, max_x;
    getmaxyx(win, max_y, max_x);
    {
      size_t glob_view_line = m_buf.GetLine(m_view_char);
      if (cursor_y < glob_view_line) {
        m_view_char = m_buf.m_front;
      } else if (cursor_y - glob_view_line + 1 >= max_y) {
        m_view_char = m_buf.FindLineStart(m_view_char);
        m_view_char += m_buf.LineLength(m_view_char) + 1;
      }
      glob_view_line = m_buf.GetLine(m_view_char);
      cursor_x_vis = cursor_x;
      cursor_y_vis = cursor_y - glob_view_line;
    }
    m_view_char = m_buf.FindLineStart(std::min(m_buf.m_front, m_view_char));
    size_t line = m_buf.GetLine(m_buf.m_front);
    size_t cursor_line_start = m_buf.FindLineStart(m_buf.m_front);
    wclear(win);
    wmove(win, 0, 0);

    cchar_t complex_char;
    for (size_t i = m_view_char; i < m_buf.m_total; i++) {
      if (i == m_buf.m_front) {
        i += m_buf.m_gap;
        if (i >= m_buf.m_total)
          break;
      }

      getyx(win, cur_y, cur_x);

      if (m_buf.m_data[i] == L'\n') {
        if (cur_y + 1 >= max_y) {
          break;
        }
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
  };

  void UpdateCursorData() {
    cursor_x = m_buf.m_front - m_buf.FindLineStart(m_buf.m_front);
    cursor_y = m_buf.GetLine(m_buf.m_front);
  }

  void HandleInput(const Mode mode, const int res, const wint_t c) {
    switch (mode) {
    case Mode::insert:
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

  void setFilepath(const std::string &path) { m_path = path; };
  bool Read() {
    if (std::filesystem::is_directory(m_path.value())) {
      m_buf.SetText(L"Cannot open directories (yet)");
      m_editable = false;
      return false;
    }
    if (!std::filesystem::exists(m_path.value())) {
      m_buf.SetText(std::format(L"File \"{}\" does not exist",
                                Utf8ToWstringICU(m_path.value())));
      m_editable = false;
      return false;
    }
    std::ifstream file(m_path.value(), std::ios::binary);
    if (!file.is_open()) {
      m_buf.SetText(std::format(L"Not able to open file \"{}\"",
                                Utf8ToWstringICU(m_path.value())));
      m_editable = false;
      return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string utf8_content = buffer.str();
    file.close();

    int32_t i = 0;
    int32_t length = static_cast<int32_t>(utf8_content.length());

    m_buf.Clean();
    while (i < length) {
      UChar32 c;
      U8_NEXT(utf8_content.c_str(), i, length, c);

      if (c < 0) {
        continue;
      }

      m_buf.insertChar(static_cast<wchar_t>(c));
    }

    m_buf.moveCursor(-m_buf.size());
    m_editable = true;
    return true;
  }
  bool Save() {
    if (!m_path) {
      return false;
    }
    std::ofstream file(m_path.value(), std::ios::binary);
    if (!file.is_open()) {
      m_buf.SetText(std::format(L"Not able to open file \"{}\"",
                                Utf8ToWstringICU(m_path.value())));
      return false;
    }
    file << WstringToUtf8ICU(m_buf.GetString());
    return true;
  }

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
