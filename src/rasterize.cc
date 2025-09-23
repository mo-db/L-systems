#include "rasterize.hpp"

// all calculations are done in worldspace
namespace viewport {
Vec2 screen_to_world(const Vec2 &vertex_screen) {
	Vec2 vertex_world = vertex_screen + vars.xy_offset;
	return vertex_world;
}
Vec2 world_to_screen(const Vec2 &vertex_world) {
	Vec2 vertex_screen = vertex_world + vars.xy_offset;
	return vertex_screen;
}
bool update_vars() { // ctrl_transform()?
	// save reference cords
	if (viewport::vars.panning_active) {
		if (app::input.ctrl.down()) {
			// add new offset to old offset
			viewport::vars.xy_offset = app::input.mouse - viewport::vars.saved_mouse + viewport::vars.xy_offset_old;
			if (!app::input.mouse_left.down()) {
				viewport::vars.panning_active = false;
			}
		} else {
			// cancel
			viewport::vars.xy_offset = viewport::vars.xy_offset_old;
			viewport::vars.panning_active = false;
		}

	// activate panning, initialize the vars
	} else if (app::input.ctrl.down() && app::input.mouse_left.down()) {
		viewport::vars.saved_mouse = app::input.mouse;
		viewport::vars.xy_offset_old = viewport::vars.xy_offset;
		viewport::vars.panning_active = true;
	}
	return 1;
}
} // namespace viewport

namespace draw {
void set_pixel(FrameBuf fb, int x, int y, uint32_t color) {
	if (x >= 0 && y >= 0 && x < fb.width && y < fb.height) {
		fb.pixels[x + y * fb.width] = color;
	}
}

void clear(FrameBuf fb, uint32_t color) {
	for (int i = 0; i < fb.width * fb.height; i++) { fb.pixels[i] = color; }
}

// Returns double the signed area but that's fine
double _edge_function(Vec2 a, Vec2 b, Vec2 c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
};


// this function needs world to screen convertion 
void bary_triangle(FrameBuf fb, Vec2 vert1_in, Vec2 vert2_in, Vec2 vert3_in, uint32_t color) {
	Vec2 vert1 = viewport::world_to_screen(vert1_in);
	Vec2 vert2 = viewport::world_to_screen(vert2_in);
	Vec2 vert3 = viewport::world_to_screen(vert3_in);

	int max_x = std::max(vert1.x, std::max(vert2.x, vert3.x));
	int min_x = std::min(vert1.x, std::min(vert2.x, vert3.x));
	int max_y = std::max(vert1.y, std::max(vert2.y, vert3.y));
	int min_y = std::min(vert1.y, std::min(vert2.y, vert3.y));
	
	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++) {

			Vec2 p{(double)x, (double)y};

			// Calculate our edge function for all three edges of the triangle ABC
			// the edge function is probably fliped because y=0 is top
			double ABP = _edge_function(vert1, vert2, p); 
			double BCP = _edge_function(vert2, vert3, p);
			double CAP = _edge_function(vert3, vert1, p);

			if (ABP >= 0 && BCP >= 0 && CAP >= 0) {
				// Point is inside the triangle ABC
				set_pixel(fb, p.x, p.y, color);
			}

		}
	}
}

// no need to world_to_screen for the lines?
// render a line as a mesh with thickness -> maybe gradient, texture, etc
// if wd (thickness) < 2, draw a thin line, gradient possible too
void wide_line(FrameBuf fb, const Line2 &line, uint32_t color, double wd) {
	if (wd < 2.0) {
		thin_line(fb, line, color);
		return;
	}
	Vec2 line_vector = line.get_v();

	// the order of vert1 and vert2 is important to get positive area
	Vec2 vert1 = line.p1 + line.get_a().get_norm() * (wd / 2.0);
	Vec2 vert2 = line.p1 - line.get_a().get_norm() * (wd / 2.0);

	// for drawing long thin lines, a higher seg count is more efficient
	// does not matter for plants though
	// if use, implement ratio of thickness to length to determine
	int seg_count = 1;
	double seg_val = 1.0 / seg_count;

	std::vector<Vec2> tri_verts{};

	// fragment line into segments
	for (int i = 0; i <= seg_count; i++) {
		// one segment
		tri_verts.push_back(vert1 + line_vector * seg_val * (double)i);
		tri_verts.push_back(vert2 + line_vector * seg_val * (double)i);
	}

	// draw the triangles
	for (int i = 0; i < seg_count * 2; i += 2) {
		bary_triangle(fb, tri_verts[i+1], tri_verts[i], tri_verts[i+2], 0xFF00FFFF);
		bary_triangle(fb, tri_verts[i+1], tri_verts[i+2], tri_verts[i+3], 0xFF0000FF);
	}
}

// this function needs world to screen convertion 
void thin_line(FrameBuf fb, const Line2 &line_world, uint32_t color) {
	// world to screen conversion
	Line2 line = {viewport::world_to_screen(line_world.p1),
								viewport::world_to_screen(line_world.p2)};
  int x0 = std::round(line.p1.x);
  int y0 = std::round(line.p1.y);
  int x1 = std::round(line.p2.x);
  int y1 = std::round(line.p2.y);

  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2; /* error value e_xy */

  for (;;) { /* loop */
    set_pixel(fb, x0, y0, color);
    e2 = 2 * err;
    if (e2 >= dy) { /* e_xy+e_x > 0 */
      if (x0 == x1)
        break;
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) { /* e_xy+e_y < 0 */
      if (y0 == y1)
        break;
      err += dx;
      y0 += sy;
    }
  }
}

// // render thick line as bresenham scanline + 2x triangles
// void thick_line(const Line2 &line, uint32_t color, double wd) {
// 	std::vector<Vec2> vertices{};
// 	vertices.push_back(line.p1 + line.get_a().get_norm() * (wd / 2.0));
// 	vertices.push_back(line.p1 - line.get_a().get_norm() * (wd / 2.0));
// 	vertices.push_back(line.p2 - line.get_a().get_norm() * (wd / 2.0));
// 	vertices.push_back(line.p2 + line.get_a().get_norm() * (wd / 2.0));
//
// 	double m = 0.0;
// 	double dx = line.p2.x - line.p1.x;
// 	double dy = line.p2.y - line.p1.y;
// 	bool m_neg = false;
//
// 	if (dy != 0.0) {
// 		m = dy / dx;
// 	}
//
// 	if (m < 0) {
// 		m_neg = true;
// 	}
//
// 	Vec2 vert1 = vertices[0];
// 	Vec2 vert2 = vertices[1];
// 	Vec2 vert3 = vertices[2];
// 	Vec2 vert4 = vertices[3];
// 	Vec2 vert5{};
// 	Vec2 vert6{};
//
// 	if (std::abs(m) > 1.0) {
// 		std::sort(vertices.begin(), vertices.end(),
// 							[](Vec2 &vertex1, Vec2 &vertex2){ return vertex1.y > vertex2.y; });
// 		// search x coordinate of vert5
// 		vert5 = {((vertices[1].y - vertices[0].y) +  m * vertices[0].x) / m, vertices[1].y};
// 		// search x coordinate of vert6
// 		vert6 = {((vertices[2].y - vertices[1].y) +  m * vertices[1].x) / m, vertices[2].y};
// 	} else {
// 		std::sort(vertices.begin(), vertices.end(),
// 							[](Vec2 &vertex1, Vec2 &vertex2){ return vertex1.x < vertex2.x; });
// 		// search y coordinate of vert5
// 		vert5 = { vertices[1].x, m * (vertices[1].x - vertices[0].x) + vertices[0].y};
// 		// search y coordinate of vert6
// 		vert6 = { vertices[2].x, m * (vertices[2].x - vertices[1].x) + vertices[1].y};
// 	}
//
// 	vert1 = vertices[0];
// 	vert2 = vertices[1];
// 	vert3 = vertices[2];
// 	vert4 = vertices[3];
//
// 	plot_circle({vert1, 10.0}, 0xFF0000FF);
// 	plot_circle({vert2, 10.0}, 0xFF00FF00);
// 	plot_circle({vert3, 10.0}, 0xFFFF0000);
// 	plot_circle({vert4, 10.0}, 0xFF00FFFF);
//
//
// 	plot_circle({vert5, 10.0}, 0xFFFFFFFF);
// 	plot_circle({vert6, 10.0}, 0xFFFFFFFF);
//
// 	// the line
// 	thin_line({vert2, vert5}, 0xFFFFFFFF);
// 	thin_line({vert2, vert6}, 0xFFFFFFFF);
// 	thin_line({vert3, vert5}, 0xFFFFFFFF);
// 	thin_line({vert3, vert6}, 0xFFFFFFFF);
//
// 	// the two triangles
// 	thin_line({vert1, vert2}, 0xFF00FFFF);
// 	thin_line({vert1, vert5}, 0xFF00FFFF);
//
// 	thin_line({vert4, vert3}, 0xFF00FFFF);
// 	thin_line({vert4, vert6}, 0xFF00FFFF);
// }

//
//
// void plot_circle(const Circle2 &circle, uint32_t color) {
// 	int xm = std::round(circle.c.x);
// 	int ym = std::round(circle.c.y);
// 	int r = std::round(circle.radius());
//   int x = -r, y = 0, err = 2 - 2 * r; /* bottom left to top right */
//   do {
// 		set_pixel(xm - x, ym + y, color); //   I. Quadrant +x +y
// 		set_pixel(xm - y, ym - x, color); //  II. Quadrant -x +y
// 		set_pixel(xm + x, ym - y, color); // III. Quadrant -x -y
// 		set_pixel(xm + y, ym + x, color); //  IV. Quadrant +x -y
//     r = err;
//     if (r <= y)
//       err += ++y * 2 + 1; /* e_xy+e_y < 0 */
//     if (r > x || err > y) /* e_xy+e_x > 0 or no 2nd y-step */
//       err += ++x * 2 + 1; /* -> x-step now */
//   } while (x < 0);
// }
} // namespace draw
