// rasterize.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"

namespace draw {
void set_pixel(int x, int y, uint32_t color);
void line(const Line2 &line, uint32_t color, double wd);
void circle(const Circle2 &circle, uint32_t color, double wd);
} // namespace draw
