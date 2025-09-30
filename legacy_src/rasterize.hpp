// rasterize.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"

namespace draw {
struct FrameBuf {
	uint32_t *pixels = nullptr;
	int width = 0;
	int height = 0;
};

void set_pixel(FrameBuf fb, int x, int y, uint32_t color);
void clear(FrameBuf fb, uint32_t color);
void bary_triangle(FrameBuf fb, Vec2 vert1, Vec2 vert2, Vec2 vert3, uint32_t color);
void wide_line(FrameBuf fb, const Line2 &line, uint32_t color, double wd);
// void thick_line(const Line2 &line, uint32_t color, double wd);
void thin_line(FrameBuf fb, const Line2 &line, uint32_t color);
void plot_circle(FrameBuf fb, const Circle2 &circle, uint32_t color);

} // namespace draw

namespace viewport {
struct Spec {
	Vec2 xy_offset{};
	Vec2 prev_mouse{};
	bool panning_active = false;
};
inline Spec spec;

State update_panning();
Vec2 screen_to_world(const Vec2 &vertex_screen);
Vec2 world_to_screen(const Vec2 &vertex_world);
} // namespace viewport
