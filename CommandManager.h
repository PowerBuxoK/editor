#pragma once
#include "Defines.h"
#include <optional>
#include <unordered_map>
#include <vector>

class Buffer;

struct Command
{
  wchar_t name;
  bool repeatable;
  bool immidiate;
  bool (*func_ptr)(Buffer* buf, const Motion& motion);
};

class CommandManager
{
public:
  CommandManager();
  ~CommandManager() = default;

  void PushCommand(wchar_t match, bool repeatable, bool immidiate, bool (*func_ptr)(Buffer* buf, const Motion& motion));
  std::optional<Command> FindCommand(wchar_t match);

private:
  std::unordered_map<wchar_t, Command> m_commands;
};
