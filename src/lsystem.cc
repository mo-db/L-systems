#include "lsystem.hpp"

namespace turtle {
Vec2 calculate_move(Turtle &turtle, const double length) {
	Vec2 position = *turtle.node;
	position.x += length * cos(turtle.angle);
	position.y += length * -sin(turtle.angle);
 	// fmt::print("pos.x: {}, pos.y: {}\n", position.x, position.y);
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
	// fmt::print("turtle.x: {}, turtle.y: {}\n", plant.turtle.node->x, plant.turtle.node->y);

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


double _eval_expr(std::string &expr_string, Lsystem &lsystem, const double in_x, const double in_y, const double in_z) {
	typedef double T; // numeric type (float, double, mpfr etc...)

	typedef exprtk::symbol_table<T> symbol_table_t;
	typedef exprtk::expression<T>   expression_t;
	typedef exprtk::parser<T>       parser_t;

	T x = T(in_x);
	T y = T(in_y);
	T z = T(in_z);
	T l = T(lsystem.vars.l);
	T m = T(lsystem.vars.m);
	// all others

	symbol_table_t symbol_table;
	symbol_table.add_variable("x",x);
	symbol_table.add_variable("y",y);
	symbol_table.add_variable("z",z);
	symbol_table.add_variable("m",m);
	symbol_table.add_variable("l",l);

	expression_t expr;
	expr.register_symbol_table(symbol_table);

	parser_t parser;

	if (!parser.compile(expr_string, expr))
	{
		 printf("Expression compilation error...\n");
		 return std::numeric_limits<double>::max();
	}

	return (T)expr.value();
}


std::string _maybe_apply_rule(Lsystem &lsystem, const char sym, const double *x_in) {
	double x{}, y{}, z{};
	if (x_in) {
		x = *x_in;	
	} else {
		if (sym == '+' || sym  == '-') {
			x = lsystem.standard_angle;
		} else {
			x = lsystem.standard_length;
		}
	}

	// check all rules
	for (auto &rule : lsystem.rules) {
		const char *letter_string = lsystem.alphabet[rule.letter_index];
		char symbol = letter_string[0];
		std::string condition = rule.condition;
		// if there is a rule for the symbol, check if condition is true
		if (symbol == sym) {
			double result = _eval_expr(condition, lsystem, x, 0.0, 0.0);
			if (util::equal_epsilon(result, 1.0)) {
				// std::vector<char> no_replace_symbols;
				std::string text = rule.text;
				std::string return_str = "";
				int cnt = 0;
				while (cnt < (int)text.size()) {
					char c = text[cnt];
					if (c == '(') {
						std::string expr_str = "";
						return_str += c;
						while ((c = text[++cnt]) != ')') {
							expr_str += c;
						}
						double expr_str_result = _eval_expr(expr_str, lsystem, x, y, z);
						fmt::print("expr_str_result: {}\n", expr_str_result);
						return_str += std::to_string(expr_str_result);
						return_str += c;
						cnt++;
					} else {
						return_str += c;
						cnt++;
					}
				}
				return return_str;
			}
		}
	}
	if (x) {
		return fmt::format("{}({})", sym, x);
	} else {
		return fmt::format("{}", sym);
	}
}

std::string generate_lstring(Lsystem &lsystem) {
	std::string lstr_expanded;
	std::string lstr = lsystem.axiom;
	for (int i = 0; i < lsystem.iterations; i++) {
		lstr_expanded = "";
		int cnt = 0;
		while (cnt < (int)lstr.size()) {
			char c = lstr[cnt];
			// if stack symbol -> copy and continue
			if (c == '[' || c == ']') {
				lstr_expanded += c;
				cnt++;
			// else process
			} else {
				// check if not last char
				if (cnt + 1 < (int)lstr.size()) {
					// () follows -> process whole string
					if (lstr[cnt+1] == '(') {
						cnt += 2; // skip bracket
						std::string value_string = "";
						while (lstr[cnt] != ')') {
							value_string += lstr[cnt++];
						}
						double value = atof(value_string.c_str());
						lstr_expanded += _maybe_apply_rule(lsystem, c, &value);
						cnt++;
						// no () follows
					} else {
						lstr_expanded += _maybe_apply_rule(lsystem, c, nullptr);
						cnt++;
					}
				// end of string follows
				} else {
					lstr_expanded += _maybe_apply_rule(lsystem, c, nullptr);
					cnt++;
				}
			}
		}
		lstr = lstr_expanded;
	}
	return lstr;
}

std::string assemble_lstring_part(Plant &plant);
} // namespace lsystem
