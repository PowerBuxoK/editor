#include "App.h"
#include "Buffer.h"
#include "CommandManager.h"
#include "Defines.h"

void RegisterDefaultCommands(CommandManager& manager)
{
  manager.PushCommand(L'a', false, true, [](Buffer* buf, const Motion& motion)
                      {
                        if(buf->m_buf[buf->m_buf.m_front] != '\n')
                          buf->m_buf.moveForward();
                        buf->m_app.m_cur_mode = Mode::insert;
                        return true;
                      });

  manager.PushCommand(L'd', true, false, [](Buffer* buf, const Motion& motion)
                      {
                        buf->m_buf.moveTo(motion.from);
                        if(motion.GetDelta() > 0)
                          buf->m_buf.deleteCharFront(motion.GetDelta());
                        else
                          buf->m_buf.deleteChar(-motion.GetDelta());
                        return true;
                      });
  manager.PushCommand(L'y', false, false, [](Buffer* buf, const Motion& motion)
                      {
                        if(!motion.valid)
                          return false;

                        size_t start = std::min(motion.from, motion.to);
                        size_t end   = std::max(motion.from, motion.to);

                        buf->m_app.m_clipboard.clear();
                        for(size_t i = start; i < end; i++)
                        {
                          buf->m_app.m_clipboard += buf->m_buf[i];
                        }
                        buf->m_app.m_cur_mode = Mode::normal;
                        return true;
                      });
}
