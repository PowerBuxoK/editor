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
    {
      size_t line_start = m_buf.FindLineStart(m_buf.m_front);
      size_t first_char = line_start;
      while(first_char < m_buf.size() && (m_buf[first_char] == L' ' || m_buf[first_char] == L'\t') && m_buf[first_char] != L'\n')
      {
        first_char++;
      }
      m_buf.moveCursor(static_cast<long long>(first_char) - static_cast<long long>(m_buf.m_front));
      m_app.m_cur_mode = Mode::insert;
    }
    break;
    case 'v':
      m_app.m_cur_mode    = Mode::visual;
      m_visual_start_char = m_buf.m_front;
      break;
    default:
      // Executed by quantifier
      for(size_t i = 0; i < quantifier; i++)
      {
        switch(macro[0])
        {
          case 'p':
            if(!m_editable)
              break;
            if(!m_app.m_clipboard.empty() && m_editable)
            {
              for(wchar_t ch : m_app.m_clipboard)
              {
                m_buf.insertChar(ch);
              }
            }
            break;
          case 'P':
            if(!m_editable)
              break;
            if(!m_app.m_clipboard.empty() && m_editable)
            {
              if(m_buf[m_buf.m_front] != '\n')
                m_buf.moveForward();
              for(wchar_t ch : m_app.m_clipboard)
              {
                m_buf.insertChar(ch);
              }
              m_buf.moveBackward();
            }
            break;
          case 'X':
            if(!m_editable)
              break;
            if(m_buf.m_front == 0 || m_buf[m_buf.m_front - 1] == L'\n')
            {
              break;
            }
            m_buf.deleteChar();
            break;
          case 'x':
            if(!m_editable)
              break;
            if(m_buf.m_front == m_buf.size() || m_buf[m_buf.m_front] == L'\n')
              break;
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
        if(m_editable && m_buf.m_front > 0)
        {
          size_t delete_pos  = m_buf.m_front - 1;
          wchar_t deleted_ch = m_buf.m_data[delete_pos < m_buf.m_front ? delete_pos : delete_pos + m_buf.m_gap];
          m_buf.deleteChar();
          RecordAction(EditActionType::Delete, delete_pos, std::wstring (1, deleted_ch));
        }
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
        if(m_editable && m_buf.m_front > 0)
        {
          size_t delete_pos  = m_buf.m_front - 1;
          wchar_t deleted_ch = m_buf.m_data[delete_pos < m_buf.m_front ? delete_pos : delete_pos + m_buf.m_gap];
          m_buf.deleteChar();
          RecordAction(EditActionType::Delete, delete_pos, std::wstring(1, deleted_ch));
        }
        break;
      default:
        if(m_editable)
        {
          size_t insert_pos = m_buf.m_front;
          m_buf.insertChar(kp.ch);
          RecordAction(EditActionType::Insert, insert_pos, std::wstring(1, kp.ch));
        }
        break;
    }
  }
  UpdateCursorData();
}
void Buffer::HandleInputVisual(const InputKeypress& kp)
{
  if(kp.type == KEY_CODE_YES)
  {
    switch(kp.ch)
    {
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
      case 'u':
        Undo();
        break;
      case 18: // Ctrl + R
        Redo();
        break;
      case 'y':
      {
        size_t start = std::min(m_visual_start_char, m_buf.m_front);
        size_t end   = std::max(m_visual_start_char, m_buf.m_front);
        m_app.m_clipboard.clear();
        for(size_t i = start; i < end; i++)
        {
          m_app.m_clipboard += m_buf[i];
        }
        m_app.m_cur_mode = Mode::normal;
      }
      break;
      case '0':
        m_buf.moveCursor(m_buf.FindLineStart(m_buf.m_front) - m_buf.m_front);
        break;
      case '$':
        m_buf.moveCursor(m_buf.LineLength(m_buf.m_front));
        break;

      case 'w':
        if(m_buf.m_front < m_buf.size())
        {
          m_buf.moveForward();
          while(m_buf.m_front < m_buf.size() && m_buf[m_buf.m_front] != L' ' && m_buf[m_buf.m_front] != L'\n')
            m_buf.moveForward();
        }
        break;
      case 'b':
        if(m_buf.m_front > 0)
        {
          m_buf.moveBackward();
          while(m_buf.m_front > 0 && m_buf[m_buf.m_front - 1] != L' ' && m_buf[m_buf.m_front - 1] != L'\n')
            m_buf.moveBackward();
        }
        break;
      case 'd':
      case 'x':
      {
        size_t start = std::min(m_visual_start_char, m_buf.m_front);
        size_t end   = std::max(m_visual_start_char, m_buf.m_front);

        m_buf.moveCursor(static_cast<long long>(end) - static_cast<long long>(m_buf.m_front));

        size_t count = end - start;
        if(count > 0 && m_editable)
        {
          std::wstring deleted_text = L"";
          for(size_t i = start; i < end; i++)
          {
            deleted_text += m_buf[i];
          }

          RecordAction(EditActionType::Delete, start, deleted_text);

          for(size_t i = 0; i < count; i++)
          {
            if(m_buf.m_front > 0)
              m_buf.deleteChar();
          }
        }
        m_app.m_cur_mode = Mode::normal;
      }
      break;

      case 'c':
      {
        size_t start = std::min(m_visual_start_char, m_buf.m_front);
        size_t end   = std::max(m_visual_start_char, m_buf.m_front);

        m_buf.moveCursor(static_cast<long long>(end) - static_cast<long long>(m_buf.m_front));

        size_t count = end - start;
        if(count > 0 && m_editable)
        {
          std::wstring deleted_text = L"";
          for(size_t i = start; i < end; i++)
          {
            deleted_text += m_buf[i];
          }

          RecordAction(EditActionType::Delete, start, deleted_text);

          for(size_t i = 0; i < count; i++)
          {
            if(m_buf.m_front > 0)
              m_buf.deleteChar();
          }
        }
        m_app.m_cur_mode = Mode::insert;
      }
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
    size_t real_idx = (i < m_buf.m_front) ? i : (i - m_buf.m_gap);

    bool is_selected = false;
    if(m_app.m_cur_mode == Mode::visual && m_app.GetCurrentWindow()->m_buf == this)
    {
      size_t start = std::min(m_visual_start_char, m_buf.m_front);
      size_t end   = std::max(m_visual_start_char, m_buf.m_front);
      if(real_idx >= start && real_idx < end)
      {
        is_selected = true;
      }
    }

    if(is_selected)
    {
      wattron(win, A_REVERSE);
    }

    setcchar(&complex_char, wch_str, WA_NORMAL, 0, NULL);
    wadd_wch(win, &complex_char);
    if(is_selected)
    {
      wattroff(win, A_REVERSE);
    }
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
    case Mode::visual:
      HandleInputVisual(kp);
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

std::wstring Buffer::getDisplayName() const
{
  if(!m_path)
  {
    return L"[No Name]";
  }
  return Utf8ToWstringICU(std::filesystem::path(m_path.value()).filename().string());
}

void Buffer::RecordAction(EditActionType type, size_t index, const std::wstring& text)
{
  m_undo_stack.push({ type, index, text });
  while(!m_redo_stack.empty())
  {
    m_redo_stack.pop();
  }
}

void Buffer::Undo()
{
  if(m_undo_stack.empty() || !m_editable)
    return;

  EditAction action = m_undo_stack.top();
  m_undo_stack.pop();

  m_buf.moveCursor(static_cast<long long>(action.index) - static_cast<long long>(m_buf.m_front));

  if(action.type == EditActionType::Insert)
  {
    m_buf.moveForward();
    for(size_t i = 0; i < action.text.size(); i++)
    {
      m_buf.deleteChar();
    }
  }
  else if(action.type == EditActionType::Delete)
  {
    for(wchar_t ch : action.text)
    {
      m_buf.insertChar(ch);
    }
  }

  m_redo_stack.push(action);
  UpdateCursorData();
}

void Buffer::Redo()
{
  if(m_redo_stack.empty() || !m_editable)
    return;

  EditAction action = m_redo_stack.top();
  m_redo_stack.pop();

  if(action.type == EditActionType::Insert)
  {
    for(wchar_t ch : action.text)
    {
      m_buf.insertChar(ch);
    }
  }
  else if(action.type == EditActionType::Delete)
  {
    m_buf.moveForward();
    for(size_t i = 0; i < action.text.size(); i++)
    {
      m_buf.deleteChar();
    }
  }

  m_undo_stack.push(action);
  UpdateCursorData();
}