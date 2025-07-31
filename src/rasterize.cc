#include "rasterize.hpp"

namespace draw {
// world to screen conversion
void set_pixel(Frame &fb, int x, int y, uint32_t color) {
	if (x >= 0 && y >= 0 && x < fb.width && y < fb.height) {
		fb.buf[x + y * fb.width] = color;
	}
}


void line(Frame &fb, const Line2 &line, uint32_t color, double wd) {
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
} // namespace draw
