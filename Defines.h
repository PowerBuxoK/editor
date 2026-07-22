#pragma once
#include <cstdint>
#include <string>

constexpr uint64_t DELTATIME = 50;

enum class Mode
{
  normal,
  insert,
  command,
  visual
};

enum class EditActionType
{
  Insert,
  Delete
};

struct Motion
{
  int GetDelta() const
  {
    return to - from;
  }
  bool valid;
  size_t from;
  size_t to;
};

struct EditAction
{
  EditActionType type;
  size_t index;
  std::wstring text;
};

struct Notification_t
{
  uint64_t time;
  std::wstring text;
};

struct InputKeypress
{
  int type;
  int ch;
  char utf8_str[5];
};
