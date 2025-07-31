// rasterize.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"

struct Frame {
	uint32_t *buf = nullptr;
	int pitch = 0;
	int width = 0;
	int height = 0;
	void clear() { buf = nullptr; pitch = width = height = 0; }
};

namespace draw {
void set_pixel(Frame &fb, int x, int y, uint32_t color);
void line(Frame &fb, const Line2 &line, uint32_t color, double wd);
} // namespace draw
