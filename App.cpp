#include "App.h"
#include "Buffer.h"
#include "Defines.h"
#include <algorithm>
#include <cstdint>
#include <curses.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

App::App() : m_manager(m_windows)
{
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Editor window
  {
    Window& v = m_windows.emplace_back();
    v.layer   = 0;

    v.m_x       = 0;
    v.m_y       = 0;
    v.m_width   = max_x;
    v.m_height  = max_y - 3;
    v.name      = L"Editor";
    Buffer& buf = m_buffers.emplace_back(*this);
    buf.m_id    = m_next_buffer_id++;
    v.m_buf     = &buf;
  }
  // Data window
  {
    m_data_window           = &m_windows.emplace_back();
    m_data_window->layer    = 0;
    m_data_window->m_x      = 0;
    m_data_window->m_y      = max_y - 3;
    m_data_window->m_width  = max_x / 8;
    m_data_window->m_height = 3;
    m_data_window->name     = L"Cursor state";
    Buffer& buf             = m_buffers.emplace_back(*this);
    buf.m_is_user_buffer    = false;
    m_data_window->m_buf    = &buf;
  }
  // Mode & command
  {
    m_mode_window           = &m_windows.emplace_back();
    m_mode_window->layer    = 0;
    m_mode_window->m_x      = max_x / 8;
    m_mode_window->m_y      = max_y - 3;
    m_mode_window->m_width  = max_x / 4;
    m_mode_window->m_height = 3;
    m_mode_window->name     = L"Mode & command";
    Buffer& buf             = m_buffers.emplace_back(*this);
    buf.m_is_user_buffer    = false;
    m_mode_window->m_buf    = &buf;
  }
  // Help
  {
    m_help_window           = &m_windows.emplace_back();
    m_help_window->layer    = 0;
    m_help_window->m_x      = max_x / 4 + max_x / 8;
    m_help_window->m_y      = max_y - 3;
    m_help_window->m_width  = max_x - max_x / 4 + max_x / 8;
    m_help_window->m_height = 3;
    m_help_window->name     = L"Help";
    Buffer& buf             = m_buffers.emplace_back(*this);
    buf.m_is_user_buffer    = false;
    m_help_window->m_buf    = &buf;
  }
  // Notification window
  {
    m_notification_window           = &m_windows.emplace_back();
    m_notification_window->layer    = 999;
    m_notification_window->m_x      = max_x - max_x / 3;
    m_notification_window->m_y      = 0;
    m_notification_window->m_width  = max_x / 3;
    m_notification_window->m_height = max_y / 3;
    m_notification_window->name     = L"Current notification";
    Buffer& buf                     = m_buffers.emplace_back(*this);
    buf.m_is_user_buffer            = false;
    m_notification_window->m_buf    = &buf;
  }
};
App::~App() {};

void App::run()
{
  while(!m_stop)
  {
    UpdateData();
    UpdateNotifications();
    // Windows stuff
    if(CheckForResize())
      RecalculateLayout();
    Draw();
    HandleInput();
    UpdateCursor();

    doupdate();
  }
};

void App::RecalculateLayout()
{
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  clear();

  // Editor window
  {
    Window& v = m_windows[0];
    v.layer   = 0;

    v.m_x      = 0;
    v.m_y      = 0;
    v.m_width  = max_x;
    v.m_height = max_y - 3;
  }
  // Data window
  {
    m_data_window->m_x      = 0;
    m_data_window->m_y      = max_y - 3;
    m_data_window->m_width  = max_x / 8;
    m_data_window->m_height = 3;
  }
  // Mode & command
  {
    m_mode_window->m_x      = max_x / 8;
    m_mode_window->m_y      = max_y - 3;
    m_mode_window->m_width  = max_x / 4;
    m_mode_window->m_height = 3;
  }
  // Help
  {
    m_help_window->m_x      = max_x / 4 + max_x / 8;
    m_help_window->m_y      = max_y - 3;
    m_help_window->m_width  = max_x - max_x / 4 + max_x / 8;
    m_help_window->m_height = 3;
  }
  // Notification window
  {
    m_notification_window->layer    = 999;
    m_notification_window->m_x      = max_x - max_x / 3;
    m_notification_window->m_y      = 0;
    m_notification_window->m_width  = max_x / 3;
    m_notification_window->m_height = max_y / 3;
  }
  Draw();
  UpdateCursor();
}

InputKeypress ReadKeypress(WINDOW* win)
{
  InputKeypress result = { 0, 0, "" };

  int c = wgetch(win);

  if(c == ERR)
    return result;

  if(c > 0xFF)
  {
    result.type = KEY_CODE_YES;
    result.ch   = c;
    return result;
  }

  result.utf8_str[0] = (char)c;
  int len            = 1;

  if((c & 0x80) != 0)
  {
    int next_ch;
    while(len < 4 && (next_ch = wgetch(win)) != ERR)
    {
      if((next_ch & 0xC0) != 0x80)
      {
        ungetch(next_ch);
        break;
      }
      result.utf8_str[len++] = (char)next_ch;
    }
  }
  result.utf8_str[len] = '\0';

  int32_t i = 0;
  U8_NEXT(result.utf8_str, i, len, result.ch);

  if(result.ch < 0)
    result.ch = (uint8_t)c;

  return result;
}

// Purely windows stuff2
bool App::CheckForResize()
{
#ifdef _WIN32
  static HANDLE hConOut = CreateFileA(
      "CONOUT$",
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL, OPEN_EXISTING, 0, NULL);

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if(hConOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConOut, &csbi))
  {
    int current_cols  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int current_lines = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    if(current_cols != COLS || current_lines != LINES)
    {
      resizeterm(current_lines, current_cols);
      return true;
    }
  }
#endif
  return false;
}

bool App::HandleSpecialKeycodes(const InputKeypress& kp)
{
  if(kp.type == KEY_CODE_YES)
  {
    switch(kp.ch)
    {
      case KEY_RESIZE:
        RecalculateLayout();
        return true;
        break;
    }
  }
  return false;
}

void App::HandleInput()
{
  InputKeypress kp = ReadKeypress(stdscr);
  if(kp.ch == 0)
    return;
  if(kp.type == ERR)
    return;
  if(HandleSpecialKeycodes(kp))
    return;
  HandleKeypress(kp);
};

void App::UpdateNotifications()
{
  if(m_notifications.size() == 0)
  {
    m_notification_window->m_buf->m_buf.SetText(L"");
    return;
  }
  m_notification_window->m_buf->m_buf.SetText(std::format(L"[Notify (1/{})]\n{}", m_notifications.size(), m_notifications.at(0).text));
  if((m_notifications.at(0).time--) <= 0)
  {
    m_notifications.pop_front();
  }
}

void App::Draw()
{
  curs_set(0);
  m_manager.Draw();
  curs_set(2);
};

void App::HandleKeypress(const InputKeypress& kp)
{
  if(m_cur_mode == Mode::normal)
  {
    HandleNormalMode(kp);
    return;
  }
  if(m_cur_mode == Mode::command)
  {
    HandleCommandMode(kp);
  }
  if(m_windows.size() <= m_focus)
    return;
  m_windows.at(m_focus).HandleInput(m_cur_mode, kp);
};

void App::HandleCommandMode(const InputKeypress& kp)
{
  if(kp.type == KEY_CODE_YES)
  {
    switch(kp.ch)
    {
      case KEY_DC:
      case KEY_BACKSPACE:
        m_command_buffer.deleteChar();
        break;
      default:
        break;
    }
    if(m_command_buffer.size() == 0)
    {
      m_cur_mode = Mode::normal;
    }
  }
  else
  {
    m_command_buffer.insertChar(kp.ch);
    TryExecuteCommand();
  }
}

void App::HandleNormalMode(const InputKeypress& kp)
{
  if(kp.type == KEY_CODE_YES)
  {
    switch(kp.ch)
    {
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
  }
  else
  {
    switch(kp.ch)
    {
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
      case 'I':
      case 'a':
      case 'x':
      case 'X':
      case 'A':
      case 'y':
      case 'p':
      case 'P':
      case 'v':
        m_command_buffer.insertChar(kp.ch);
        TryExecuteMacro();
        break;
      case ':':
        if(m_command_buffer.size() == 0)
          m_command_buffer.insertChar(kp.ch);
        m_cur_mode = Mode::command;
        break;
    }
  }
}

void App::TryExecuteCommand()
{
  auto cmd = m_command_buffer.GetString().substr(1);
  if(cmd.size() > 0 && cmd.at(cmd.size() - 1) == L'\n')
  {
    cmd = m_command_buffer.GetString().substr(1, cmd.size() - 1);
    m_command_buffer.Clean();
    m_cur_mode        = Mode::normal;
    auto command_main = cmd.find(L" ") == cmd.npos ? cmd : cmd.substr(0, cmd.find(L" "));
    auto command_arg  = cmd.find(L" ") == cmd.npos ? L"" : cmd.substr(cmd.find(L" ") + 1);
    auto success      = HandleCommand(command_main, command_arg);
    SendNotification(success.size() < 10 ? 1000 : 3000, L"{}", success);
    last_cmd = std::format(L":CMD: {} {}", command_main, command_arg,
                           success);
  }
};

std::wstring App::HandleCommand(const std::wstring& cmd, const std::wstring& arg)
{
  if(cmd == L"w")
  {
    if(m_windows.size() <= m_focus)
    {
      return L"No buffer!";
    }

    if(arg.size() != 0)
    {
      m_windows.at(m_focus).m_buf->setFilepath(WstringToUtf8ICU(arg));
    }
    if(m_windows.at(m_focus).m_buf->Save())
    {
      return L"SUCCESS";
    }
    else
    {
      return L"FAIL";
    }
  }
  if(cmd == L"o")
  {
    if(m_windows.size() <= m_focus)
    {
      return L"No buffer!";
    }

    Buffer& new_buf = m_buffers.emplace_back(*this);
    new_buf.m_id    = m_next_buffer_id++;
    new_buf.setFilepath(WstringToUtf8ICU(arg));
    bool ok = new_buf.Read();

    m_windows.at(m_focus).m_buf = &new_buf;

    return ok ? L"SUCCESS" : L"FAIL";
  }
  if(cmd == L"q")
  {
    m_stop = true;
    return L"Quitting!";
  }
  if(cmd == L"pwd")
  {
    return std::format(L"{}", std::filesystem::current_path().wstring());
  }
  if(cmd == L"bn" || cmd == L"bp")
  {
    if(m_windows.size() <= m_focus)
    {
      return L"No window!";
    }

    auto user_buffers = GetUserBuffers();
    if(user_buffers.empty())
    {
      return L"No buffers open";
    }

    auto it    = std::find(user_buffers.begin(), user_buffers.end(),
                           m_windows.at(m_focus).m_buf);
    size_t idx = (it != user_buffers.end())
                     ? std::distance(user_buffers.begin(), it)
                     : 0;

    size_t new_idx = (cmd == L"bn")
                         ? (idx + 1) % user_buffers.size()
                         : (idx + user_buffers.size() - 1) % user_buffers.size();

    m_windows.at(m_focus).m_buf = user_buffers[new_idx];
    return L"SUCCESS";
  }
  if(cmd == L"bl")
  {
    if(m_windows.size() <= m_focus)
    {
      return L"No window!";
    }
    auto user_buffers = GetUserBuffers();
    if(user_buffers.empty())
    {
      return L"No buffers open";
    }
    std::wstring m_to_send = L"Buffers:\n";

    uint64_t cid = 0;
    for(auto& buf : user_buffers)
    {
      cid++;
      m_to_send += std::format(L"{}: {} {}\n", cid, buf->getDisplayName(), m_windows[m_focus].m_buf == buf ? L"[C]" : L"");
    }
    return m_to_send;
  }

  return L"No cmd found";
}

void App::TryExecuteMacro()
{
  if(m_windows.size() <= m_focus)
  {
    m_command_buffer.Clean();
    return;
  }
  auto cmd          = m_command_buffer.GetString();
  size_t quantifier = 1;
  try
  {
    quantifier = std::stoull(cmd);
  }
  catch(std::invalid_argument)
  {
  }
  size_t first_non_quantifier = cmd.size();
  for(size_t i = 0; i < cmd.size(); i++)
  {
    if(!std::isdigit(cmd[i]))
    {
      first_non_quantifier = i;
      break;
    }
  }
  auto to_exec  = cmd.substr(first_non_quantifier);
  bool executed = m_windows.at(m_focus).m_buf->HandleMacro(quantifier, to_exec);
  if(executed)
  {
    last_cmd = m_command_buffer.GetString();
    m_command_buffer.Clean();
  }
};

void App::PushCommandBuffer(const std::wstring& str)
{
  for(auto& v : str)
  {
    m_command_buffer.insertChar(v);
  }
}

void App::OpenFile(std::string path)
{
  Buffer& new_buf = m_buffers.emplace_back(*this);
  new_buf.m_id    = m_next_buffer_id++;
  new_buf.setFilepath(path);
  new_buf.Read();

  if(m_windows.size() <= m_focus)
  {
    return;
  }
  m_windows.at(m_focus).m_buf = &new_buf;
};

void App::UpdateData()
{
  if(m_windows.size() <= m_focus)
    return;
  Window& cur_wnd = m_windows.at(m_focus);

  std::wstring buf_line;
  {
    auto user_buffers = GetUserBuffers();
    Window& cur_wnd   = m_windows.at(m_focus);
    auto it           = std::find(user_buffers.begin(), user_buffers.end(), cur_wnd.m_buf);

    if(it != user_buffers.end())
    {
      size_t idx = std::distance(user_buffers.begin(), it);
      size_t n   = user_buffers.size();

      buf_line = std::format(L"[{}]", user_buffers[idx]->getDisplayName());

      if(n > 1)
      {
        size_t next_idx = (idx + 1) % n;
        buf_line        = std::format(L"{}  {}", buf_line, user_buffers[next_idx]->getDisplayName());
      }
      if(n > 2)
      {
        size_t prev_idx = (idx + n - 1) % n;
        buf_line        = std::format(L"{}  {}", user_buffers[prev_idx]->getDisplayName(), buf_line);
      }
    }
    m_data_window->m_buf->m_buf.SetText(
        std::format(L"{}:{}\nW:{}", cur_wnd.m_buf->getCursorY(),
                    cur_wnd.m_buf->getCursorX(), cur_wnd.name));
  }

  std::wstring cur_mode;
  switch(m_cur_mode)
  {
    case Mode::insert:
      cur_mode = L"INSERT";
      m_help_window->m_buf->m_buf.SetText(L"HELP [INSERT MODE]: \n"
                                          "Type to insert text. Press ESC to return to normal mode.\n"
                                          "Backspace to delete characters. Use arrow keys to navigate.");
      break;
    case Mode::command:
      cur_mode = L"COMMAND";
      m_help_window->m_buf->m_buf.SetText(L"HELP [COMMAND MODE]: \n"
                                          ":w [filename] - save :q - quit\n"
                                          ":o [filename] - open");
      break;
    case Mode::normal:
      m_help_window->m_buf->m_buf.SetText(
          L"HELP [NORMAL MODE]: \n"
          "hjkl  -  move a - append A - append at EOL : - "
          "command mode\n"
          "arrows - move i - insert I - Insert at SOL");
      cur_mode = L"NORMAL";

      break;
    case Mode::visual:
      cur_mode = L"VISUAL";
      m_help_window->m_buf->m_buf.SetText(L"HELP [VISUAL MODE]: \n"
                                          "hjkl / arrows - select text. ESC - return to normal mode.\n"
                                          "y - copy selected text.");
      break;
    default:
      m_help_window->m_buf->m_buf.SetText(
          L"HELP : Not avaliable for current mode\n");
  };
  auto vis_cmd = m_command_buffer.GetString();
  if(vis_cmd.size() == 0)
  {
    vis_cmd = last_cmd;
  }

  m_mode_window->m_buf->m_buf.SetText(
      std::format(L"{:10} : {}", cur_mode, vis_cmd));
};

Window* App::GetCurrentWindow()
{
  if(m_windows.size() <= m_focus)
  {
    return nullptr;
  }
  return &m_windows[m_focus];
}

void App::UpdateCursor()
{
  if(m_windows.size() <= m_focus)
    return;
  Window& cur_wnd = m_windows.at(m_focus);
  cur_wnd.m_buf->UpdateCursorData();

  WINDOW* win;
  win = newwin(cur_wnd.m_y, cur_wnd.m_x, cur_wnd.m_height, cur_wnd.m_width);
  move(cur_wnd.m_buf->visualCursorY() + cur_wnd.m_y, cur_wnd.m_buf->visualCursorX() + cur_wnd.m_x);
  wnoutrefresh(win);
  delwin(win);
};

std::vector<Buffer*> App::GetUserBuffers()
{
  std::vector<Buffer*> result;
  for(auto& b : m_buffers)
  {
    if(b.m_is_user_buffer)
    {
      result.emplace_back(&b);
    }
  }
  return result;
}
