#include "CommandManager.h"
#include "Buffer.h"
#include <optional>

CommandManager::CommandManager() {};

void CommandManager::PushCommand(wchar_t match, bool repeatable, bool immidiate, bool (*func_ptr)(Buffer* buf, const Motion& motion))
{
  m_commands.try_emplace(match, std::move(Command{ match, repeatable, immidiate, func_ptr }));
};

std::optional<Command> CommandManager::FindCommand(wchar_t match)
{
  if(m_commands.find(match) != m_commands.end())
    return m_commands.at(match);
  else
    return std::nullopt;
};
