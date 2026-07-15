#include "Manager.h"

Manager::Manager(std::deque<Window>& windows) : m_windows(windows) {};
Manager::~Manager() {};

void Manager::Draw()
{
  std::vector<Window*> windows_sorted;
  windows_sorted.reserve(m_windows.size());

  for(int i = 0; i < m_windows.size(); i++)
  {
    windows_sorted.emplace_back(&m_windows[i]);
  }

  for(int i = 0; i < m_windows.size(); i++)
  {
    for(int j = 0; j < m_windows.size(); j++)
    {
      if(windows_sorted[i]->layer < windows_sorted[j]->layer)
      {
        std::swap(windows_sorted[i], windows_sorted[j]);
      }
    }
  }

  for(auto& v : windows_sorted)
  {
    v->Draw();
  }
};
