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
};
