#include "lsystem.hpp"

namespace turtle {
void move(Turtle &turtle, const double length) {
	turtle.p.x += length * cos(turtle.angle);
	turtle.p.y += length * -sin(turtle.angle);
}
void turn(Turtle &turtle, const double angle) { turtle.angle += angle; }
} // namespace turtle


namespace lsystem {
void _turtle_action(Turtle &turtle, std::vector<Turtle> &turtle_stack,
                   Plant &plant, Lsystem &lsystem, const char c,
                   const double *value) {
	double x;

	// grow branch
	if (std::isalpha(c)) {
		bool visable = true;
		if (std::islower(c)) {
			visable = false;
		}
		if (value) {
			x = *value * lsystem.standard_length;
		} else {
			x = lsystem.standard_length;
		}
    turtle::move(turtle, x);
		plant.add_node(turtle.p);
    plant.branches.push_back(Branch{turtle.current_node, plant.nodes.back().id, c, visable, 1.0});
		turtle.current_node = plant.nodes.back().id;
	}

	// turn turtle counter-clockwise
	if (c == '-') {
		if (value) {
			x = *value;	
		} else {
			x = lsystem.standard_angle;
		}
    turtle::turn(turtle, -x);
	}

	// turn turtle clockwise
	if (c == '+') {
		if (value) {
			x = *value;	
		} else {
			x = lsystem.standard_angle;
		}
    turtle::turn(turtle, x);
	}

	// push and pop turtle state
	if (c == '[') {
    turtle_stack.push_back(turtle);
	}
  if (c == ']') {
    turtle = turtle_stack.back();
    turtle_stack.pop_back();
	}
}

Plant generate_plant(Turtle &turtle, std::vector<Turtle> &turtle_stack, Lsystem &lsystem, const Vec2 start, const std::string lstring) {
	Plant plant{}; 
	plant.add_node(start);
	turtle.current_node = plant.nodes.back().id;

	int counter = 0;
	while (counter < (int)lstring.size()) {
		char c = lstring[counter];

		if (c == '[' || c == ']') {
			_turtle_action(turtle, turtle_stack, plant, lsystem, c, nullptr);
			counter++;
		}

		if (std::isalpha(c)) {
			// check if not last char
			if (counter + 1 < (int)lstring.size()) {
				// () follows -> process whole string
				if (lstring[counter+1] == '(') {
					counter += 2; // skip bracket
					std::string value_string = "";
					while (lstring[counter] != ')') {
						value_string += lstring[counter++];
					}
					double value = atof(value_string.c_str());
					_turtle_action(turtle, turtle_stack, plant, lsystem, c, &value);
					counter++;
					// no () follows
				} else {
					_turtle_action(turtle, turtle_stack, plant, lsystem, c, nullptr);
					counter++;
				}
			// end of string follows
			} else {
				_turtle_action(turtle, turtle_stack, plant, lsystem, c, nullptr);
				counter++;
			}
		}

		}

	return plant;
}
std::string generate_lstring(Lsystem &lsystem);
std::string assemble_lstring_part(Plant &plant);
} // namespace lsystem
