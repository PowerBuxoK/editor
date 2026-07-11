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

class Buffer
{
public:
  Buffer(App& app) : m_buf(), m_app(app) {};
  ~Buffer() {};

  void Draw(WINDOW* win);
  void UpdateCursorData();
  void HandleInput(const Mode mode, const InputKeypress& kp);
  bool HandleMacro(const size_t quantifier, const std::wstring& macro);
  void HandleInputInsert(const InputKeypress& kp);
  void HandleInputVisual(const InputKeypress& kp);

  size_t getCursorX() const { return cursor_x; }
  size_t getCursorY() const { return cursor_y; }

  size_t visualCursorX() const { return cursor_x_vis; }
  size_t visualCursorY() const { return cursor_y_vis; }

  void setFilepath(const std::string& path) { m_path = path; };
  bool Read();
  bool Save();

  GapBuffer m_buf;
  bool m_is_user_buffer = true;
  size_t m_id           = 0;

  std::wstring getDisplayName() const;
  
private:
  size_t cursor_x     = 0;
  size_t cursor_y     = 0;
  size_t cursor_x_vis = 0;
  size_t cursor_y_vis = 0;
  size_t m_view_char  = 0;
  std::optional<std::string> m_path;
  App& m_app;
  bool m_enable_wrapping = false;
  bool m_editable        = true;
  size_t m_visual_start_char = 0;
};
