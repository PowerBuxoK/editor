#include "Buffer.h"
#include "App.h"
#include "Defines.h"

bool Buffer::HandleMacro(const size_t quantifier, const std::wstring& macro)
{
  if(macro.size() <= 0)
  {
    return false;
  }
  // One-letter macros
  switch(macro[0])
  {
      // Executed one time
    case 'a':
      if(m_buf[m_buf.m_front] != '\n')
        m_buf.moveForward();
      m_app.m_cur_mode = Mode::insert;
      break;
    case 'A':
      m_buf.moveCursor(m_buf.LineLength(m_buf.m_front));
      m_app.m_cur_mode = Mode::insert;
      break;
    case 'i':
      m_app.m_cur_mode = Mode::insert;
      break;
    case 'I':
      m_buf.moveCursor(m_buf.FindLineStart(m_buf.m_front) - m_buf.m_front);
      m_app.m_cur_mode = Mode::insert;
      break;
    default:
      // Executed by quantifier
      for(size_t i = 0; i < quantifier; i++)
      {
        switch(macro[0])
        {
          case 'X':
            if(m_buf.m_front == 0 || m_buf[m_buf.m_front - 1] == L'\n')
            {
              break;
            }
            m_buf.deleteChar();
            break;
          case 'x':
            if(m_buf.m_front == m_buf.size() || m_buf[m_buf.m_front] == L'\n')
            {
              break;
            }
            m_buf.moveForward();
            m_buf.deleteChar();
            break;
          case 'h':
            m_buf.moveBackward();
            break;
          case 'l':
            m_buf.moveForward();
            break;
          case 'j':
            m_buf.moveDown(cursor_x);
            break;
          case 'k':
            m_buf.moveUp(cursor_x);
            break;
        }
      }
  }
  UpdateCursorData();
  return true;
};

void Buffer::HandleInputInsert(const InputKeypress& kp)
{
  if(kp.type == KEY_CODE_YES)
  {
    switch(kp.ch)
    {
      case KEY_BACKSPACE:
      case KEY_DC:
        if(m_editable)
          m_buf.deleteChar();
        break;
      case KEY_UP:
        m_buf.moveUp(cursor_x);
        break;
      case KEY_DOWN:
        m_buf.moveDown(cursor_x);
        break;
      case KEY_LEFT:
        m_buf.moveBackward();
        break;
      case KEY_RIGHT:
        m_buf.moveForward();
        break;
    }
  }
  else
  {
    switch(kp.ch)
    {
      case 27:
        m_app.m_cur_mode = Mode::normal;
        break;
      case 127:
      case 8:
        if(m_editable)
          m_buf.deleteChar();
        break;
      default:
        if(m_editable)
          m_buf.insertChar(kp.ch);
        break;
    }
  }
  UpdateCursorData();
}
void Buffer::Draw(WINDOW* win)
{
  int cur_y, cur_x, max_y, max_x;
  getmaxyx(win, max_y, max_x);
  {
    size_t glob_view_line = m_buf.GetLine(m_view_char);
    if(cursor_y < glob_view_line)
    {
      m_view_char = m_buf.m_front;
    }
    else if(cursor_y - glob_view_line + 1 >= max_y)
    {
      m_view_char = m_buf.FindLineStart(m_view_char);
      m_view_char += m_buf.LineLength(m_view_char) + 1;
    }
    glob_view_line = m_buf.GetLine(m_view_char);
    cursor_x_vis   = cursor_x;
    cursor_y_vis   = cursor_y - glob_view_line;
  }
  m_view_char              = m_buf.FindLineStart(std::min(m_buf.m_front, m_view_char));
  size_t line              = m_buf.GetLine(m_buf.m_front);
  size_t cursor_line_start = m_buf.FindLineStart(m_buf.m_front);
  wclear(win);
  wmove(win, 0, 0);

  cchar_t complex_char;
  for(size_t i = m_view_char; i < m_buf.m_total; i++)
  {
    if(i == m_buf.m_front)
    {
      i += m_buf.m_gap;
      if(i >= m_buf.m_total)
        break;
    }

    getyx(win, cur_y, cur_x);

    if(m_buf.m_data[i] == L'\n')
    {
      if(cur_y + 1 >= max_y)
      {
        break;
      }
      wmove(win, cur_y + 1, 0);
      continue;
    }

    wchar_t wch_str[2] = { m_buf.m_data[i], L'\0' };
    if(!m_enable_wrapping)
    {
      if(cur_x >= max_x - 1)
      {
        continue;
      }

      if(cur_x >= max_x - 2)
      {
        wch_str[0] = L'>';
      }
    }

    setcchar(&complex_char, wch_str, WA_NORMAL, 0, NULL);
    wadd_wch(win, &complex_char);
  }
};

void Buffer::UpdateCursorData()
{
  cursor_x = m_buf.m_front - m_buf.FindLineStart(m_buf.m_front);
  cursor_y = m_buf.GetLine(m_buf.m_front);
};

void Buffer::HandleInput(const Mode mode, const InputKeypress& kp)
{
  switch(mode)
  {
    case Mode::insert:
      HandleInputInsert(kp);
      break;
    default:
      break;
  }
};

bool Buffer::Read()
{
  if(std::filesystem::is_directory(m_path.value()))
  {
    m_buf.SetText(L"Cannot open directories (yet)");
    m_editable = false;
    return false;
  }
  if(!std::filesystem::exists(m_path.value()))
  {
    m_buf.SetText(std::format(L"File \"{}\" does not exist",
                              Utf8ToWstringICU(m_path.value())));
    m_editable = false;
    return false;
  }
  std::ifstream file(m_path.value(), std::ios::binary);
  if(!file.is_open())
  {
    m_buf.SetText(std::format(L"Not able to open file \"{}\"",
                              Utf8ToWstringICU(m_path.value())));
    m_editable = false;
    return false;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string utf8_content = buffer.str();
  file.close();

  int32_t i      = 0;
  int32_t length = static_cast<int32_t>(utf8_content.length());

  m_buf.Clean();
  while(i < length)
  {
    UChar32 c;
    U8_NEXT(utf8_content.c_str(), i, length, c);

    if(c < 0)
    {
      continue;
    }

    m_buf.insertChar(static_cast<wchar_t>(c));
  }

  m_buf.moveCursor(-m_buf.size());
  m_editable = true;
  return true;
};

bool Buffer::Save()
{
  if(!m_path)
  {
    return false;
  }
  std::ofstream file(m_path.value(), std::ios::binary);
  if(!file.is_open())
  {
    m_buf.SetText(std::format(L"Not able to open file \"{}\"",
                              Utf8ToWstringICU(m_path.value())));
    return false;
  }
  file << WstringToUtf8ICU(m_buf.GetString());
  return true;
};
