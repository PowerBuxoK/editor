#pragma once
enum class Mode
{
  normal,
  insert,
  command
};

struct InputKeypress
{
  int type;
  int ch;
  char utf8_str[5];
};
