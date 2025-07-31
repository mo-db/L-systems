#pragma once
#include "core.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

struct Turtle {
  double x = 0.0;
  double y = 0.0;
  double angle = gk::pi / 2;
  Turtle() = default;
  Turtle(const double x, const double y, const double angle)
      : x{x}, y{y}, angle{angle} {}
};

namespace turtle {
void move(Turtle &turtle, double length);
void turn(Turtle &turtle, double angle);
} // namespace turtle
