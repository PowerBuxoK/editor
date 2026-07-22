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
  manager.PushCommand(L'x', true, true, [](Buffer* buf, const Motion& motion)
                      {
                        if(!buf->m_editable)
                          return false;

                        size_t start = std::min(motion.from, motion.to);
                        size_t end   = std::max(motion.from, motion.to);
                        if(start == end && start < buf->m_buf.size())
                        {
                          end = start + 1;
                        }

                        size_t count = end - start;
                        if(count > 0)
                        {
                          std::wstring deleted_text = L"";
                          for(size_t i = start; i < end; i++)
                          {
                            deleted_text += buf->m_buf[i];
                          }

                          buf->RecordAction(EditActionType::Delete, start, deleted_text);

                          buf->m_buf.moveTo(start);
                          for(size_t i = 0; i < count; i++)
                          {
                            buf->m_buf.deleteCharFront();
                          }
                        }
                        buf->m_app.m_cur_mode = Mode::normal;
                        return true;
                      });
  manager.PushCommand(L'c', true, false, [](Buffer* buf, const Motion& motion)
                      {
                        if(!motion.valid || !buf->m_editable)
                          return false;

                        size_t start = std::min(motion.from, motion.to);
                        size_t end   = std::max(motion.from, motion.to);
                        size_t count = end - start;

                        if(count > 0)
                        {
                          std::wstring deleted_text = L"";
                          for(size_t i = start; i < end; i++)
                          {
                            deleted_text += buf->m_buf[i];
                          }

                          buf->RecordAction(EditActionType::Delete, start, deleted_text);

                          buf->m_buf.moveTo(start);

                          buf->m_buf.deleteCharFront(count);
                        }
                        buf->m_app.m_cur_mode = Mode::insert;
                        return true;
                      });
  manager.PushCommand(L'p', true, true, [](Buffer* buf, const Motion& motion)
                      {
                        if(!buf->m_editable || buf->m_app.m_clipboard.empty())
                          return false;

                        size_t insert_pos = buf->m_buf.m_front;
                        buf->RecordAction(EditActionType::Insert, insert_pos, buf->m_app.m_clipboard);

                        for(wchar_t ch : buf->m_app.m_clipboard)
                        {
                          buf->m_buf.insertChar(ch);
                        }
                        return true;
                      });
  manager.PushCommand(L'v', false, false, [](Buffer* buf, const Motion& motion)
                      {
                        buf->m_app.m_cur_mode    = Mode::visual;
                        buf->m_visual_start_char = buf->m_buf.m_front;

                        return true;
                      });
}
