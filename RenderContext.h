#pragma once
#include "defines.h"
#include <cstdint>
#include <unicode/unistr.h>
#include <vector>

namespace ed {
class RenderContext {
  uint32_t width, height;

public:
  RenderContext();
  ~RenderContext();

  void DrawText(const point_t &pos);
};
} // namespace ed
