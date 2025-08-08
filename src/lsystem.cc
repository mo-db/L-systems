#include "lsystem.hpp"
#include "core.hpp"

namespace lsystem {

// returns string between brackets, index must sit on '('
std::optional<std::string> _get_string_between_brackets(std::string in_string, const int index) {
	assert(in_string[index] == '(');

	// set offset_index to the first char of the string
	int offset_index = index + 1;	
	std::string expr_string = "";

	// fill expr_string
  bool bracket_closes;
  while ((bracket_closes = (offset_index < (int)in_string.size())) &&
         in_string[offset_index] != ')') {
    expr_string += in_string[offset_index++];
  }

	// only return expr_string if there was a closing bracket
  if (bracket_closes) {
    return expr_string;
  } else {
		return {};
  }
}

Vec2 _calculate_move(Turtle &turtle, const double length) {
	Vec2 position = *turtle.node;
	position.x += length * cos(turtle.angle);
	position.y += length * -sin(turtle.angle);
 	// fmt::print("pos.x: {}, pos.y: {}\n", position.x, position.y);
	return position;
}
void _turn(Turtle &turtle, const double angle) { turtle.angle += angle; }

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

		// check nodes limit
		if (plant.node_counter < plant.MAX_NODES) {
			Vec2 *last_node = turtle.node;
			Vec2 *new_node = plant.add_node(_calculate_move(turtle, x));
			turtle.node = new_node;
    	plant.branches.push_back(Branch{last_node, new_node, c, visable, 1.0});
		} else {
			std::puts("node limit reached");
		}
	}

	// turn turtle counter-clockwise
	if (c == '-') {
		if (value) {
			x = *value;	
		} else {
			x = lsystem.standard_angle;
		}
    _turn(turtle, -x);
	}

	// turn turtle clockwise
	if (c == '+') {
		if (value) {
			x = *value;	
		} else {
			x = lsystem.standard_angle;
		}
    _turn(turtle, x);
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
	Plant plant{}; // plant muss parameter sein
	plant.turtle.node = plant.add_node(start);

	int index = 0;
	while (index < (int)lstring.size()) {
		char c = lstring[index];

		if (c == '[' || c == ']') {
			_turtle_action(plant, lsystem, c, nullptr);
			index++;
		} else {

			// check c is the last character in the string
			if (index + 1 < (int)lstring.size()) {

				if (lstring[index + 1] == '(') {
					if (auto result = _get_string_between_brackets(lstring, index + 1)) {
						std::string expr_string = result.value();
						double expr_value = atof(expr_string.c_str());
						_turtle_action(plant, lsystem, c, &expr_value);
						index++;
					} else {
						_turtle_action(plant, lsystem, c, nullptr);
						index++;
					}

				} else {
					_turtle_action(plant, lsystem, c, nullptr);
					index++;
				}

			} else {
				_turtle_action(plant, lsystem, c, nullptr);
				index++;
			}

		}
	}
	return plant;
}


std::optional<double> _eval_expr(std::string &expr_string, Lsystem &lsystem, const double in_x) {
	typedef double T; // numeric type (float, double, mpfr etc...)

	typedef exprtk::symbol_table<T> symbol_table_t;
	typedef exprtk::expression<T>   expression_t;
	typedef exprtk::parser<T>       parser_t;

	T x = T(in_x);
	// T y = T(in_y);
	// T z = T(in_z);
	T l = T(lsystem.parameters[0]);
	T m = T(lsystem.parameters[1]);
	T n = T(lsystem.parameters[2]);
	T o = T(lsystem.parameters[3]);
	// all others

	symbol_table_t symbol_table;
	symbol_table.add_variable("x",x);
	// symbol_table.add_variable("y",y);
	// symbol_table.add_variable("z",z);
	symbol_table.add_variable("l",l);
	symbol_table.add_variable("m",m);
	symbol_table.add_variable("n",n);
	symbol_table.add_variable("o",o);

	expression_t expr;
	expr.register_symbol_table(symbol_table);

	parser_t parser;
	if (!parser.compile(expr_string, expr)) {
		fmt::print("Expression compile error...\n");
		return {};
	}

	return (T)expr.value();
}


// cannot fail
std::string _maybe_apply_rule(Lsystem &lsystem, const char symbol, const double *x_in) {
	// if no value specified in braces, set fitting default
	double x{};
	if (x_in) {
		x = *x_in;	
	} else {
		if (symbol == '+' || symbol  == '-') {
			x = lsystem.standard_angle;
		} else {
			x = lsystem.standard_length;
		}
	}

	// iterate rules
	for (auto &rule : lsystem.rules) {
		char rule_symbol = (lsystem.alphabet[rule.symbol_index])[0];
		std::string condition = rule.condition;
		std::string text = rule.text;

		// if there is a rule with fitting symbol, check if condition is true
		if (rule_symbol == symbol) {

			// eval condition, and update FIELD_STATE based on outcome
			if (auto result = _eval_expr(condition, lsystem, x)) {
				if (util::equal_epsilon(result.value(), 1.0)) {
					rule.condition_state = Lsystem::FIELD_STATE::TRUE;
				} else {
					rule.condition_state = Lsystem::FIELD_STATE::FALSE;
				}
			} else {
				rule.condition_state = Lsystem::FIELD_STATE::ERROR;
			}

			// if condition is met, substitute rule for symbol, try eval all expr
			if (rule.condition_state == Lsystem::FIELD_STATE::TRUE) {
				std::string return_str = "";
				int index = 0;

				while (index < (int)text.size()) {
					char c = text[index];
					// check if c is expr -> eval

					if (c == '(') {
						// if expr_string valid, evaluate and add to return_string

						if (auto result = _get_string_between_brackets(text, index)) {
							std::string expr_string = result.value();
							// set to 0.0 if expr cant eval fails
							double expr_value = 
								_eval_expr(expr_string, lsystem, x).value_or(0.0);
							// add the brackets with the evaluated expr_string to return_string
							return_str += '(';
							return_str += fmt::format("{}", expr_value);
							return_str += ')';
							// move index to position after closing bracket
							index += (int)expr_string.size() + 2;

						// expr_string not valid
						} else {
							index++;
						}

					// add c to expr_string and inc index
					} else {
						return_str += c;
						index++;
					}
				} // while end

				return return_str;
			}
		}
	}

	// return the following if no rule could match
	if (x) {
		return fmt::format("{}({})", symbol, x);
	} else {
		return fmt::format("{}", symbol);
	}
}

// execute this on parameter text change
bool eval_parameters(Lsystem &lsystem) {
	for (int i = 0; i < lsystem.n_parameters; i++) {
		std::string parameter_string = lsystem.parameter_strings[i];
		if (auto result = _eval_expr(parameter_string, lsystem, 0.0)) {
			lsystem.parameters[i] = result.value();
		} else {
			return false;
		}
	}
	return true;
}

std::string generate_lstring(Lsystem &lsystem) {
	std::string lstring_expanded;
	std::string lstring = lsystem.axiom.text;
	for (int i = 0; i < lsystem.iterations; i++) {
		lstring_expanded = "";
		int index = 0;
		while (index < (int)lstring.size()) {
			char c = lstring[index];

			// if stack symbol -> copy and continue
			if (c == '[' || c == ']') {
				lstring_expanded += c;
				index++;
			} else {

				// check c is the last character in the string
				if (index + 1 < (int)lstring.size()) {

					// check if the next character is an open bracket
					if (lstring[index + 1] == '(') {
						if (auto result = _get_string_between_brackets(lstring, index+1)) {
							std::string expr_string = result.value();
							double expr_value =
								_eval_expr(expr_string, lsystem, 0.0).value_or(0.0);

							lstring_expanded += _maybe_apply_rule(lsystem, c, &expr_value);
							// move index to position after closing bracket
							index += (int)expr_string.size() + 3;
						} else {
							lstring_expanded += _maybe_apply_rule(lsystem, c, nullptr);
							index++;
						}

					} else {
						lstring_expanded += _maybe_apply_rule(lsystem, c, nullptr);
						index++;
					}

				} else {
					lstring_expanded += _maybe_apply_rule(lsystem, c, nullptr);
					index++;
				}

			}
		}
		lstring = lstring_expanded;
	}
	return lstring;
}

// TODO serialize lsystem
bool save_rule_as_file(Lsystem::Rule &rule, const std::string &save_file_name) {
	std::string save_file_full_path = app::context.save_path + '/' + save_file_name;

	fmt::print("save file: {}\n", save_file_full_path);

	FILE* fp = std::fopen(save_file_full_path.c_str(), "w");
	if (!fp) {
		std::puts("std::fopen failed");
		return false;
	}

	bool is_ok = true;
	constexpr int n_items = app::gui.textfield_size;
  if (fwrite(rule.condition, sizeof(rule.condition[0]), n_items, fp) != n_items) {
		is_ok = false;
	}
	if (fwrite(rule.text, sizeof(rule.text[0]), n_items, fp) != n_items) {
		is_ok = false;
	}

	fclose(fp);

	// delete file if frwite() failed
	if (!is_ok) {
		assert(std::remove(save_file_full_path.c_str()) == 0);
	}
	return true;
}


// TODO load lsystem from file
bool load_rule_from_file(Lsystem::Rule &rule, std::string &save_file_name) {
	std::string save_file = app::context.save_path + '/' + save_file_name;

	FILE* fp = std::fopen(save_file.c_str(), "r");
	if (!fp) {
		std::puts("std::fopen failed");
		return false;
	}

	constexpr int n_items = app::gui.textfield_size;
	char temp_buf[n_items];
  if (std::fread(temp_buf, sizeof(temp_buf[0]), n_items, fp) == n_items) {
		std::memcpy(rule.condition, temp_buf, n_items * sizeof(temp_buf[0]));
	} else {
		std::fclose(fp);
		return false;
	}
  if (std::fread(temp_buf, sizeof(temp_buf[0]), n_items, fp) == n_items) {
		std::memcpy(rule.text, temp_buf, n_items * sizeof(temp_buf[0]));
	} else {
		std::fclose(fp);
		return false;
}

	std::fclose(fp);
	return true;
}


std::optional<std::vector<std::string>> scan_saves() {
	namespace fs = std::filesystem;
	std::string save_path = app::context.save_path;

	std::vector<std::string> names;
	if (fs::exists(save_path) && fs::is_directory(save_path)) {
			for (auto const& entry : fs::directory_iterator(save_path)) {
					if (entry.is_regular_file()) {
							names.push_back(entry.path().filename().string());
					}
			}
	}
	std::sort(names.begin(), names.end());
	if (names.size() > 0) {
		return names;
	} else {
		return {};
	}
}
} // namespace lsystem
