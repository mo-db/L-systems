// rasterize.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"

namespace draw {
void set_pixel(App &app, int x, int y, uint32_t color);
void line(App &app, const Line2 &line, uint32_t color, double wd);
void circle(App &app, const Circle2 &circle, uint32_t color, double wd);
} // namespace draw
