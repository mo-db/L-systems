#include "rasterize.hpp"

namespace draw {
// world to screen conversion
void set_pixel(App &app, int x, int y, uint32_t color) {
	if (x >= 0 && y >= 0 && x < app.video.width && y < app.video.height) {
		app.video.frame_buf[x + y * app.video.width] = color;
	}
}


void line(App &app, const Line2 &line, uint32_t color, double wd) {
  int x0 = std::round(line.p1.x);
  int y0 = std::round(line.p1.y);
  int x1 = std::round(line.p2.x);
  int y1 = std::round(line.p2.y);

  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2; /* error value e_xy */

  for (;;) { /* loop */
    set_pixel(app, x0, y0, color);
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


void plot_circle(App &app, uint32_t *pixel_buf, const Circle2 &circle, uint32_t color) {
	int xm = std::round(circle.c.x);
	int ym = std::round(circle.c.y);
	int r = std::round(circle.radius());
  int x = -r, y = 0, err = 2 - 2 * r; /* bottom left to top right */
  do {
		set_pixel(app, xm - x, ym + y, color); //   I. Quadrant +x +y
		set_pixel(app, xm - y, ym - x, color); //  II. Quadrant -x +y
		set_pixel(app, xm + x, ym - y, color); // III. Quadrant -x -y
		set_pixel(app, xm + y, ym + x, color); //  IV. Quadrant +x -y
    r = err;
    if (r <= y)
      err += ++y * 2 + 1; /* e_xy+e_y < 0 */
    if (r > x || err > y) /* e_xy+e_x > 0 or no 2nd y-step */
      err += ++x * 2 + 1; /* -> x-step now */
  } while (x < 0);
}
} // namespace draw
