#pragma once

#include "Buffer.h"
#include "Defines.h"
#include "GapBuffer.h"
#include "Manager.h"
#include "UTFHelpers.h"
#include "Window.h"
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <curses.h>
#include <deque>
#include <format>
#include <ncurses.h>
#include <string>

class App
{
public:
  App();
  ~App();
  void run();
  void HandleInput();
  void Draw();
  void HandleKeypress(const int res, const wint_t c);
  void HandleCommandMode(const int res, const wint_t c);
  void HandleNormalMode(const int res, const wint_t c);
  void TryExecuteCommand();
  std::wstring HandleCommand(const std::wstring &cmd, const std::wstring &arg);
  void TryExecuteMacro();
  void PushCommandBuffer(const std::wstring &str);
  void OpenFile(std::string path);
  void UpdateData();
  void UpdateCursor();

  friend bool Buffer::HandleMacro(const size_t quantifier,
                                  const std::wstring &macro);
  friend void Buffer::HandleInputInsert(const int res, const wint_t c);

private:
  Mode m_cur_mode = Mode::normal;
  uint64_t m_focus = 0;
  bool m_stop = false;
  std::deque<Buffer> m_buffers;
  std::deque<Window> m_windows;
  Window *m_data_window = nullptr;
  Window *m_mode_window = nullptr;
  Window *m_help_window = nullptr;
  std::wstring last_cmd;
  GapBuffer m_command_buffer;
  Manager m_manager;
};