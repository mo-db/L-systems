// rasterize.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"

namespace draw {
void set_pixel(int x, int y, uint32_t color);
void thick_line_mesh(const Line2 &line, uint32_t color, double wd, int seg_count);
void thick_line(const Line2 &line, uint32_t color, double wd);
void thin_line(const Line2 &line, uint32_t color);
void plot_circle(const Circle2 &circle, uint32_t color);
} // namespace draw
