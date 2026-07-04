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

class App {
public:
  App() : m_manager(m_windows) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Editor window
    {
      Window &v = m_windows.emplace_back();
      v.layer = 0;

      v.m_x = 0;
      v.m_y = 0;
      v.m_width = max_x;
      v.m_height = max_y - 3;
      v.name = L"Editor";
      Buffer &buf = m_buffers.emplace_back(*this);
      v.m_buf = &buf;
    }
    // Data window
    {
      m_data_window = &m_windows.emplace_back();
      m_data_window->layer = 0;
      m_data_window->m_x = 0;
      m_data_window->m_y = max_y - 3;
      m_data_window->m_width = max_x / 8;
      m_data_window->m_height = 3;
      m_data_window->name = L"Cursor state";
      Buffer &buf = m_buffers.emplace_back(*this);
      m_data_window->m_buf = &buf;
    }
    // Mode & command
    {
      m_mode_window = &m_windows.emplace_back();
      m_mode_window->layer = 0;
      m_mode_window->m_x = max_x / 8;
      m_mode_window->m_y = max_y - 3;
      m_mode_window->m_width = max_x / 4;
      m_mode_window->m_height = 3;
      m_mode_window->name = L"Mode & command";
      Buffer &buf = m_buffers.emplace_back(*this);
      m_mode_window->m_buf = &buf;
    }
    // Help
    {
      m_help_window = &m_windows.emplace_back();
      m_help_window->layer = 0;
      m_help_window->m_x = max_x / 4 + max_x / 8;
      m_help_window->m_y = max_y - 3;
      m_help_window->m_width = max_x - max_x / 4 + max_x / 8;
      m_help_window->m_height = 3;
      m_help_window->name = L"Help";
      Buffer &buf = m_buffers.emplace_back(*this);
      m_help_window->m_buf = &buf;
    }
  };
  ~App() {};

  void run() {
    while (!m_stop) {
      UpdateData();
      Draw();
      UpdateCursor();
      HandleInput();
      UpdateCursor();
    }
  };

  void HandleInput() {
    wint_t c;
    int res = get_wch(&c);
    if (res == ERR)
      return;
    HandleKeypress(res, c);
  };

  void Draw() {
    switch (m_cur_mode) {
    case Mode::normal:
      m_help_window->m_buf->m_buf.SetText(
          L"HELP [NORMAL MODE]: \n"
          "hjkl  -  move a - append A - append at EOL : - "
          "command mode\n"
          "arrows - move i - insert I - [nop]");
      break;
    case Mode::command:
      m_help_window->m_buf->m_buf.SetText(L"HELP [COMMAND MODE]: \n"
                                          ":w [filename] - save :q - quit\n"
                                          ":o [filename] - open");
      break;
    default:
      m_help_window->m_buf->m_buf.SetText(
          L"HELP : Not avaliable for current mode\n");
    }
    m_manager.Draw();
  };

  void HandleKeypress(const int res, const wint_t c) {
    if (m_cur_mode == Mode::normal) {
      HandleNormalMode(res, c);
      return;
    }
    if (m_cur_mode == Mode::command) {
      HandleCommandMode(res, c);
    }
    if (m_windows.size() <= m_focus)
      return;
    m_windows.at(m_focus).HandleInput(m_cur_mode, res, c);
  };

  void HandleCommandMode(const int res, const wint_t c) {
    if (res == KEY_CODE_YES) {
      switch (c) {
      case KEY_DC:
      case KEY_BACKSPACE:
        m_command_buffer.deleteChar();
        break;
      default:
        break;
      }
      if (m_command_buffer.size() == 0) {
        m_cur_mode = Mode::normal;
      }
    } else {
      m_command_buffer.insertChar(c);
      TryExecuteCommand();
    }
  }

  void HandleNormalMode(const int res, const wint_t c) {
    if (res == KEY_CODE_YES) {
      switch (c) {
      case KEY_UP:
        PushCommandBuffer(L"k");
        break;
      case KEY_DOWN:
        PushCommandBuffer(L"j");
        break;
      case KEY_DC:
      case KEY_LEFT:
        PushCommandBuffer(L"h");
        break;
      case KEY_RIGHT:
        PushCommandBuffer(L"l");
        break;
      }
      TryExecuteMacro();
    } else {
      switch (c) {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0':
      case 'h':
      case 'j':
      case 'k':
      case 'l':
      case 'i':
      case 'a':
      case 'x':
      case 'X':
      case 'A':
        m_command_buffer.insertChar(c);
        TryExecuteMacro();
        break;
      case ':':
        if (m_command_buffer.size() == 0)
          m_command_buffer.insertChar(c);
        m_cur_mode = Mode::command;
        break;
      }
    }
  }

  void TryExecuteCommand() {
    auto cmd = m_command_buffer.GetString().substr(1);
    if (cmd.size() > 0 && cmd.at(cmd.size() - 1) == L'\n') {
      cmd = m_command_buffer.GetString().substr(1, cmd.size() - 1);
      m_command_buffer.Clean();
      m_cur_mode = Mode::normal;
      auto command_main =
          cmd.find(L" ") == cmd.npos ? cmd : cmd.substr(0, cmd.find(L" "));
      auto command_arg =
          cmd.find(L" ") == cmd.npos ? L"" : cmd.substr(cmd.find(L" ") + 1);
      auto success = HandleCommand(command_main, command_arg);
      last_cmd = std::format(L":CMD: {} {} \n[{}]", command_main, command_arg,
                             success);
    }
  };

  std::wstring HandleCommand(const std::wstring &cmd, const std::wstring &arg) {
    if (cmd == L"w") {
      if (m_windows.size() <= m_focus) {
        return L"No buffer!";
      }

      if (arg.size() != 0) {
        m_windows.at(m_focus).m_buf->setFilepath(WstringToUtf8ICU(arg));
      }
      if (m_windows.at(m_focus).m_buf->Save()) {
        return L"SUCCESS";
      } else {
        return L"FAIL";
      }
    }
    if (cmd == L"o") {
      if (m_windows.size() <= m_focus) {
        return L"No buffer!";
      }

      m_windows.at(m_focus).m_buf->setFilepath(WstringToUtf8ICU(arg));
      if (m_windows.at(m_focus).m_buf->Read()) {
        return L"SUCCESS";
      } else {
        return L"FAIL";
      }
    }
    if (cmd == L"q") {
      m_stop = true;
      return L"Quitting!";
    }
    if (cmd == L"pwd") {
      return std::format(L"{}", std::filesystem::current_path().wstring());
    }
    return L"No cmd found";
  }

  void TryExecuteMacro() {
    if (m_windows.size() <= m_focus) {
      m_command_buffer.Clean();
      return;
    }
    auto cmd = m_command_buffer.GetString();
    size_t quantifier = 1;
    try {
      quantifier = std::stoull(cmd);
    } catch (std::invalid_argument) {
    }
    size_t first_non_quantifier = cmd.size();
    for (size_t i = 0; i < cmd.size(); i++) {
      if (!std::isdigit(cmd[i])) {
        first_non_quantifier = i;
        break;
      }
    }
    auto to_exec = cmd.substr(first_non_quantifier);
    bool executed =
        m_windows.at(m_focus).m_buf->HandleMacro(quantifier, to_exec);
    if (executed) {
      last_cmd = m_command_buffer.GetString();
      m_command_buffer.Clean();
    }
  };

  void PushCommandBuffer(const std::wstring &str) {
    for (auto &v : str) {
      m_command_buffer.insertChar(v);
    }
  }

  void OpenFile(std::string path) {};

  void UpdateData() {
    if (m_windows.size() <= m_focus)
      return;
    Window &cur_wnd = m_windows.at(m_focus);
    m_data_window->m_buf->m_buf.SetText(
        std::format(L"{}:{}\nW:{}", cur_wnd.m_buf->getCursorY(),
                    cur_wnd.m_buf->getCursorX(), cur_wnd.name));

    std::wstring cur_mode;
    switch (m_cur_mode) {
    case Mode::insert:
      cur_mode = L"INSERT";
      break;
    case Mode::command:
      cur_mode = L"COMMAND";
      break;
    case Mode::normal:
      cur_mode = L"NORMAL";
      break;
    };
    auto vis_cmd = m_command_buffer.GetString();
    if (vis_cmd.size() == 0) {
      vis_cmd = last_cmd;
    }

    m_mode_window->m_buf->m_buf.SetText(
        std::format(L"{:10} : {}", cur_mode, vis_cmd));
  };

  void UpdateCursor() {
    if (m_windows.size() <= m_focus)
      return;
    Window &cur_wnd = m_windows.at(m_focus);
    cur_wnd.m_buf->UpdateCursorData();
    move(cur_wnd.m_buf->visualCursorY(), cur_wnd.m_buf->visualCursorX());
  };

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
