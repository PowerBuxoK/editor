#pragma once

#include "Buffer.h"
#include "CommandManager.h"
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
  void HandleKeypress(const InputKeypress& kp);
  void HandleCommandMode(const InputKeypress& kp);
  void HandleNormalMode(const InputKeypress& kp);
  bool HandleSpecialKeycodes(const InputKeypress& kp);
  bool CheckForResize();
  void TryExecuteCommand();
  std::wstring HandleCommand(const std::wstring& cmd, const std::wstring& arg);
  void TryExecuteMacro();
  void RecalculateLayout();
  void PushCommandBuffer(const std::wstring& str);
  void OpenFile(std::string path);
  void UpdateData();
  void UpdateCursor();
  void UpdateNotifications();
  Window* GetCurrentWindow();
  template <typename... Args>
  void SendNotification(uint64_t time, std::wstring_view rt_fmt_str, Args&&... args)
  {
    const auto nots = std::format(L"{}", std::vformat(rt_fmt_str, std::make_wformat_args(args...)));

    m_notifications.emplace_back(std::move(Notification_t{ time / DELTATIME, nots }));
  }

  friend bool Buffer::HandleMacro(const size_t quantifier,
                                  const std::wstring& macro);
  friend void Buffer::HandleInputInsert(const InputKeypress& kp);
  friend void Buffer::HandleInputVisual(const InputKeypress& kp);

  std::wstring m_clipboard;
  Mode m_cur_mode = Mode::normal;

private:
  uint64_t m_focus = 0;
  bool m_stop      = false;
  std::deque<Buffer> m_buffers;
  std::deque<Window> m_windows;
  Window* m_data_window         = nullptr;
  Window* m_mode_window         = nullptr;
  Window* m_help_window         = nullptr;
  Window* m_notification_window = nullptr;
  std::wstring last_cmd;
  GapBuffer m_command_buffer;
  CommandManager m_cmd_manager;
  Manager m_manager;
  int max_x = 0, max_y = 0;
  size_t m_next_buffer_id = 0;
  std::vector<Buffer*> GetUserBuffers();

  std::deque<Notification_t> m_notifications;
};
