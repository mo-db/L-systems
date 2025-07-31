#include "turtle.hpp"

namespace turtle {
void move(Turtle &turtle, double length) {
	turtle.x += length * cos(turtle.angle);
	turtle.y += length * -sin(turtle.angle);
}
void turn(Turtle &turtle, double angle) { turtle.angle += angle; }
} // namespace turtle
