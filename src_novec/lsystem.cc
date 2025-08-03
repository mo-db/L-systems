#include "lsystem.hpp"

namespace turtle {
Vec2 calculate_move(Turtle &turtle, const double length) {
	Vec2 position = *turtle.node;
	position.x += length * cos(turtle.angle);
	position.y += length * -sin(turtle.angle);
 	fmt::print("pos.x: {}, pos.y: {}\n", position.x, position.y);
	return position;
}
void turn(Turtle &turtle, const double angle) { turtle.angle += angle; }
} // namespace turtle


namespace lsystem {
void _turtle_action(Plant &plant, Lsystem &lsystem, const char c,
                   const double *value) {
	double x;
	Turtle &turtle = plant.turtle;
	std::vector<Turtle> &turtle_stack = plant.turtle_stack;

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

		Vec2 *last_node = turtle.node;
		Vec2 *new_node = plant.add_node(turtle::calculate_move(turtle, x));
		turtle.node = new_node;
    plant.branches.push_back(Branch{last_node, new_node, c, visable, 1.0});
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

Plant generate_plant(Lsystem &lsystem, const Vec2 start, const std::string lstring) {
	Plant plant{};
	plant.turtle.node = plant.add_node(start);
	fmt::print("turtle.x: {}, turtle.y: {}\n", plant.turtle.node->x, plant.turtle.node->y);

	int counter = 0;
	while (counter < (int)lstring.size()) {
		char c = lstring[counter];

		if (c == '[' || c == ']') {
			_turtle_action(plant, lsystem, c, nullptr);
			counter++;
		} else {
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
					_turtle_action(plant, lsystem, c, &value);
					counter++;
					// no () follows
				} else {
					_turtle_action(plant, lsystem, c, nullptr);
					counter++;
				}
			// end of string follows
			} else {
				_turtle_action(plant, lsystem, c, nullptr);
				counter++;
			}
		}
	}

	return plant;
}
std::string generate_lstring(Lsystem &lsystem);
std::string assemble_lstring_part(Plant &plant);
} // namespace lsystem
