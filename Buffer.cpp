#include "Buffer.h"
#include "App.h"
#include "Defines.h"

bool Buffer::HandleMacro(const size_t quantifier, const std::wstring &macro) {
  if (macro.size() <= 0) {
    return false;
  }
  // One-letter macros
  switch (macro[0]) {
    // Executed one time
  case 'a':
    if (m_buf[m_buf.m_front] != '\n')
      m_buf.moveForward();
    m_app.m_cur_mode = Mode::insert;
    break;
  case 'A':
    m_buf.moveCursor(m_buf.FindLineStart(m_buf.m_front) - m_buf.m_front +
                     m_buf.LineLength(m_buf.m_front));
    m_app.m_cur_mode = Mode::insert;
    break;
  case 'i':
    m_app.m_cur_mode = Mode::insert;
    break;
  case 'q':
    m_app.m_stop = true;
    break;
  default:
    // Executed by quantifier
    for (size_t i = 0; i < quantifier; i++) {
      switch (macro[0]) {
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
  // if (res == KEY_CODE_YES) {
  //   switch (c) {
  //   case KEY_UP:
  //     m_buf.moveUp(cursor_x);
  //     break;
  //   case KEY_DOWN:
  //     m_buf.moveDown(cursor_x);
  //     break;
  //   case KEY_DC:
  //   case KEY_LEFT:
  //     m_buf.moveBackward();
  //     break;
  //   case KEY_RIGHT:
  //     m_buf.moveForward();
  //     break;
  //   }
  // } else {
  //   switch (c) {
  //   case 'i':
  //     m_app.m_cur_mode = Mode::insert;
  //     break;
  //   case 'q':
  //     m_app.m_stop = true;
  //     break;
  //   }
  // }
  UpdateCursorData();
  return true;
};

void Buffer::HandleInputInsert(const int res, const wint_t c) {
  if (res == KEY_CODE_YES) {
    switch (c) {
    case KEY_BACKSPACE:
    case KEY_DC:
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
  } else {
    switch (c) {
    case 27:
      m_app.m_cur_mode = Mode::normal;
      break;
    case 127:
    case 8:
      m_buf.deleteChar();
      break;
    default:
      m_buf.insertChar(c);
      break;
    }
  }
  UpdateCursorData();
}
