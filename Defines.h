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
