#pragma once
enum class Mode
{
  normal,
  insert,
  command,
  visual
};

struct InputKeypress
{
  int type;
  int ch;
  char utf8_str[5];
};
