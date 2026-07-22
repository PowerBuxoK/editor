#include "GapBuffer.h"

GapBuffer::GapBuffer() : m_total(64), m_front(0), m_gap(64)
{
  m_data = (wchar_t*)malloc(m_total * sizeof(wchar_t));
  if(!m_data)
  {
    std::cout << "Failed to create gap buffer!" << std::endl;
    exit(-1);
  }
}

GapBuffer::~GapBuffer()
{
  if(m_data)
    free(m_data);
}

void GapBuffer::moveCursor(int delta)
{
  if(delta == 0)
    return;

  size_t len;
  wchar_t *dst, *src;

  if(delta < 0)
  {
    len = -delta;
    if(len > m_front)
      len = m_front;
    if(len == 0)
      return;
    dst = m_data + m_front + m_gap - len;
    src = m_data + m_front - len;
    m_front -= len;
  }
  else
  {
    size_t back = m_total - m_front - m_gap;
    len         = delta;
    if(len > back)
      len = back;
    if(len == 0)
      return;
    dst = m_data + m_front;
    src = m_data + m_front + m_gap;
    m_front += len;
  }

  memmove(dst, src, len * sizeof(wchar_t));
}

void GapBuffer::moveTo(size_t id)
{
  if(id >= size())
    return;

  moveCursor(static_cast<int>(id) - static_cast<int>(m_front));
};

void GapBuffer::grow(size_t size)
{
  if(m_gap >= size)
    return;

  size_t old_total = m_total;

  while(m_gap < size)
  {
    m_total *= 2;
    m_gap = m_total - old_total + m_gap;
  }

  m_data = (wchar_t*)realloc(m_data, m_total * sizeof(wchar_t));
  if(!m_data)
  {
    std::cout << "Failed to reallocate gap buffer!" << std::endl;
    exit(-1);
  }

  memmove(m_data + m_front + m_gap,
          m_data + m_front + (m_gap - (m_total - old_total)),
          (old_total - m_front - (m_gap - (m_total - old_total))) * sizeof(wchar_t));
}

void GapBuffer::insertChar(wchar_t c)
{
  if(m_gap == 0)
    grow(1);

  m_data[m_front] = c;
  m_front++;
  m_gap--;
}

void GapBuffer::deleteChar(size_t count)
{
  count = std::min(m_front, count);
  m_front -= count;
  m_gap += count;
}

void GapBuffer::deleteCharFront(size_t count)
{
  count = std::min(count, size() - m_front);
  m_gap += count;
}

void GapBuffer::moveForward()
{
  if(m_total - m_front - m_gap == 0)
    return;
  m_data[m_front] = m_data[m_front + m_gap];
  m_front++;
}

void GapBuffer::moveBackward()
{
  if(m_front == 0)
    return;
  m_front--;
  m_data[m_front + m_gap] = m_data[m_front];
}

void GapBuffer::moveUp(size_t cursor_x)
{
  size_t cur_line_start = FindLineStart(m_front);
  if(cur_line_start == 0)
    return;
  size_t prev_line_start = FindLineStart(cur_line_start - 1);
  size_t prev_line_len   = LineLength(prev_line_start);

  size_t target_x = std::min(cursor_x, prev_line_len);
  moveCursor(static_cast<int>(prev_line_start + target_x) - static_cast<int>(m_front));
}

void GapBuffer::moveDown(size_t cursor_x)
{
  size_t cur_line_start  = FindLineStart(m_front);
  size_t next_line_start = cur_line_start + LineLength(cur_line_start) + 1;
  if(next_line_start >= size())
    return;
  size_t next_line_len = LineLength(next_line_start);

  size_t target_x = std::min(cursor_x, next_line_len);
  moveCursor(static_cast<int>(next_line_start + target_x) - static_cast<int>(m_front));
}

size_t GapBuffer::LineLength(size_t id)
{
  size_t len = 0;
  while(id < size() && (*this)[id] != L'\n')
  {
    len++;
    id++;
  }
  return len;
}

size_t GapBuffer::FindLineStart(size_t id)
{
  while(id > 0)
  {
    if((*this)[id - 1] == L'\n')
      break;
    id--;
  }
  return id;
}

size_t GapBuffer::GetLine(size_t id)
{
  size_t count = 1;
  while((id--) > 0)
  {
    if((*this)[id] == L'\n')
      count++;
  }
  return count;
}

void GapBuffer::Clean()
{
  m_front = 0;
  m_gap   = m_total;
}

size_t GapBuffer::size() { return m_total - m_gap; }

void GapBuffer::SetText(const std::wstring& str)
{
  grow(str.size());
  m_front = 0;
  m_gap   = m_total - str.size();
  memcpy(m_data + m_gap, str.c_str(), str.size() * sizeof(wchar_t));
}

std::wstring GapBuffer::GetString()
{
  std::wstring data;
  for(size_t i = 0; i < m_total; i++)
  {
    if(i == m_front)
    {
      i += m_gap;
      if(i >= m_total)
        break;
    }
    data += m_data[i];
  }
  return data;
}
