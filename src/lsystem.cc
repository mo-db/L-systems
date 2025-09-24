#include "lsystem_new.hpp"
#include "core.hpp"

namespace lsystem_new {

State Module::update_vars() {
  for (int i = global_vars.size() - 1; i >= 0; i--) {
    Var &var = global_vars[i];
    if (*(var.expr) == '\0') {
      continue;
    }
    if (var.use_slider) {
    } else {
      std::string glob_var_str = var.expr;
      var.value = evaluate_expression(
				glob_var_str, nullptr, nullptr, nullptr).value_or(0.f);
    }
  }
  return State::True;
}

std::optional<float> Module::evaluate_expression(
		std::string &expression_string, float *x, float *y, float *z) 
{
  typedef float T;
  typedef exprtk::symbol_table<T> symbol_table_t;
  typedef exprtk::expression<T> expression_t;
  typedef exprtk::parser<T> parser_t;
  symbol_table_t symbol_table;

  // add symbol args to symbol table, if given
  if (x) {
    symbol_table.add_variable("x", *x);
  }
  if (y) {
    symbol_table.add_variable("y", *y);
  }
  if (z) {
    symbol_table.add_variable("z", *z);
  }

  // add global vars to symbol table
  for (int i = global_vars.size() - 1; i >= 0; i--) {
    Var &var = global_vars[i];
    symbol_table.add_variable(var.label, var.value);
  }

  // parse expression
  expression_t expression;
  expression.register_symbol_table(symbol_table);
  parser_t parser;
  if (!parser.compile(expression_string, expression)) {
    fmt::print("Expression compile error...\n");
    fmt::print("expr_string: {}\n", expression_string);
    return {};
  }
  return (T)expression.value();
}

void update_vars(Module &module) {
	for (int i = module.global_vars.size() - 1; i >= 0; i--) {
		Var &var = module.global_vars[i];
		if (*(var.expr) == '\0') { continue; }
		if (var.use_slider) {
		} else {
			std::string glob_var_str = var.expr;
			var.value = _eval_expr(module, glob_var_str, nullptr, nullptr, nullptr).value_or(0.0);
		}
	}
}

Vec2 _calculate_move(Plant &plant, const float length) {
	Turtle &turtle = plant.turtle;
	Vec2 position = plant.nodes[turtle.node_id];
	position.x += length * cos(turtle.angle);
	position.y += length * -sin(turtle.angle);
	return position;
}
void _turn(Turtle &turtle, const float angle) { turtle.angle += angle; }


// this could return possible defaults for all 3 vars
std::optional<float> get_default(Module &module, const char symbol) {
	if (std::isalpha(symbol)) {
		return module.get_default_var(DefaultVar::move).value;
	}
	if (symbol == '-' || symbol == '+') {
		return module.get_default_var(DefaultVar::rotate).value;
	}
	// TODO
	if (symbol == '%' || symbol == '^') {
		return module.get_default_var(DefaultVar::width).value;
	}
	print_info("lstring contains undefined symbol");
	return {};
}

// if args are empty returns default or 0.0
std::array<float, 3> symbol_eval_args(Module &module, const char symbol, const std::string &args_str) {
	auto result = get_default(module, symbol);
	if (result == std::nullopt) { return {}; }
	float x_default = result.value();

	std::array<float, 3> defaults{ x_default, 0.0, 0.0 };

	Args args{};
 	int n_args = 0;
	if (args_str.empty()) {
		return defaults;
	} else {
 		n_args = parse_args(args_str, args);
		if (n_args == 0) { 
			print_info("parse_args error");
		}
	}

	// x,y,z values to return
	std::array<float, 3> values{ 0.0, 0.0, 0.0 };

	// if x or default: n_args = 1, if x,y n_args = 2, and then 3
	for (int i = n_args - 1; i >= 0; i--) {
		float &value = values[i];
		// depending on the current arg querry allready calculated values
		float *x, *y, *z;
		// i could make x,y,z a vector<> size n_args
		x = nullptr;
		if (i == 2) {
			y = nullptr;
			z = nullptr;
		} else if (i == 1) {
			y = nullptr;
			if (n_args == 2) {
				z = &values[2];
			} else {
				z = nullptr;
			}
		} else if (i == 0) {
			if (n_args == 1) {
				y = &values[1];
			} else {
				y = nullptr;
			}
			if (n_args == 2) {
				z = &values[2];
			} else {
				z = nullptr;
			}
		}

		std::string base = "";
		std::string pattern = "";
		std::vector<std::string> repeats{};
		std::string scale = "";
		if (!parse_arg(args[i], base, pattern, repeats, scale)) {
			print_info("arg parse error");
		}

		if (base.empty()) {
			value += defaults[i];
		} else {

			value += _eval_expr(module, base, x, y, z).value_or(0.0);
		}

		// at the moment ignore pattern

		// now repeats
		if (!repeats.empty()) {
			for (auto &repeat : repeats) {
				char repeat_op;
				std::string repeat_expr;
				int repeat_n = 1; // default to 1 if no n specified
				if (!parse_block(repeat, repeat_op, repeat_expr, &repeat_n)) {
					print_info("repeat parse error");
				}

				if (repeat_op == '+') {
					value += _eval_expr(module, repeat_expr, x, y, z).value_or(0.0) * repeat_n;
				}
				if (repeat_op == '-') {
					value -= _eval_expr(module, repeat_expr, x, y, z).value_or(0.0) * repeat_n;
				}
				if (repeat_op == '*') {
					value *= std::pow(_eval_expr(module, repeat_expr, x, y, z).value_or(1.0), repeat_n);
				}
				if (repeat_op == '/') {
					value /= std::pow(_eval_expr(module, repeat_expr, x, y, z).value_or(1.0), repeat_n);
				}
			}
		}
	}

	return values;
}

// this will not only be turtle actions but also thickness, color etc changes
void _turtle_action(Module &module, const char symbol, const std::string args) {
	Plant &plant = module.plant;
	Turtle &turtle = plant.turtle;
	std::vector<Turtle> &turtle_stack = plant.turtle_stack;

	std::array<float, 3> args_ary{};
	if (symbol != '[' && symbol != ']') {
		args_ary = symbol_eval_args(module, symbol, args);
	}
	// TODO why dont i use y and z here?
	float x = args_ary[0];
	float y = args_ary[1];
	float z = args_ary[2];

	// grow branch
	if (std::isalpha(symbol)) {
		bool visable = true;
		if (std::islower(symbol)) {
			visable = false;
		}
		plant.grow(_calculate_move(plant, x));
	}

	// turn turtle counter-clockwise
	if (symbol == '-') {
    _turn(turtle, -x);
	}
	// turn turtle clockwise
	if (symbol == '+') {
    _turn(turtle, x);
	}

	// TODO
	// this should either set or change the width
	if (symbol == '%') {
		turtle.width += 1;
	}

	// push and pop turtle state
	if (symbol == '[') {
    turtle_stack.push_back(turtle);
	}
  if (symbol == ']') {
    turtle = turtle_stack.back();
    turtle_stack.pop_back();
	}
}

// plant start is 0.0 allways?
bool generate_plant_timed(Module &module){
	std::string &lstring = module.lstring;
	Plant &plant = module.plant;
	int &current_index = plant.current_lstring_index;

	int index = current_index;
	int accum = 0;
	while (index < lstring.size()) {

		// check if 60 percent of time elapsed since frame start, then return
		if (++accum >= 5) { // how often to check
			accum = 0;
			util::ms elapsed = util::Clock::now() - app::context.frame_start;
			// print_info(fmt::format("generate_plant elapsed: {}\n",
			// 			(elapsed / app::context.frame_time)));
			if ((elapsed / app::context.frame_time) >= 0.6) {
				current_index = index;
				return false;
			}
		}

		char c = lstring[index];

		// because of that, the check in the while condition is redundant
		if (index + 1 >= lstring.size()) {
			_turtle_action(module, c, "");
			current_index = 0;
			return true;
		}
		else if (lstring[index + 1] != '<') {
			_turtle_action(module, c, "");
			index++;
		}
		else {
			index += 2; // move to after '<'
			// fmt::print("[DEBUG] lstring[index]: {}\n", lstring[index]);
			std::string args = util::get_substr(lstring, index, '>');
			if (args.empty()) {
				print_info("lstring invalid");
				current_index = 0;
				return true;
			} else {
				_turtle_action(module, c, args);
				index += args.size() + 1; // move to after '>'
			}
		}
	}

	current_index = 0;
	return true;
}

State LsystemManager::draw_plants_timed(draw::FrameBuf &fb) {
	int accum = 0;

	for (int i = lsystem_new::plants_drawn; i < modules.size(); i++) {
		auto &plant = get_module(i)->plant;

		for (int i = plant.current_branch; i < plant.branches.size(); i++) {
			print_info(fmt::format("current branch {}\n", i));

			// ---- timing ----
			if (++accum >= 10) {
				accum = 0;
				util::ms elapsed = util::Clock::now() - app::context.frame_start;
				// print_info(fmt::format("draw_plant elapsed: {}\n",
				// 			(elapsed / app::context.frame_time)));
				if ((elapsed / app::context.frame_time) >= 0.9) {
					plant.current_branch = i;
					return State::False;
				}
			}

			auto &branch = plant.branches[i];
			const Vec2 &node1 = plant.nodes[branch.node1_id];
			const Vec2 &node2 = plant.nodes[branch.node2_id];
			draw::wide_line(fb, Line2{node1, node2}, plant.color, branch.wd);
		}
		plant.current_branch = 0;
		lsystem_new::plants_drawn++;
	}
	return State::True;
}

std::optional<float> _eval_expr(Module module, std::string &expr_string, float *x,
																 float *y, float *z) {
	typedef float T;
	typedef exprtk::symbol_table<T> symbol_table_t;
	typedef exprtk::expression<T>   expression_t;
	typedef exprtk::parser<T>       parser_t;
	symbol_table_t symbol_table;

	// add symbol args to symbol table, if given
	if (x) { symbol_table.add_variable("x", *x); }
	if (y) { symbol_table.add_variable("y", *y); }
	if (z) { symbol_table.add_variable("z", *z); }

	// add global vars to symbol table
	for (int i = module.global_vars.size() - 1; i >= 0; i--) {
		Var &var = module.global_vars[i];
		symbol_table.add_variable(var.label, var.value);
	}

	// parse expression
	expression_t expr;
	expr.register_symbol_table(symbol_table);
	parser_t parser;
	if (!parser.compile(expr_string, expr)) {
		fmt::print("Expression compile error...\n");
		fmt::print("expr_string: {}\n", expr_string);
		return {};
	}
	return (T)expr.value();
}

int parse_args(const std::string &args_str, Args &args) {
	if (args_str.empty()) { return 0; }
  int n_args = 1;
  for (int i = 0; i < args_str.size(); i++) {
    if (args_str[i] == ';') {
      n_args++;
    } else {
			if (n_args == 1) {
				args.x += args_str[i];
			} else if (n_args == 2) {
				args.y += args_str[i];
			} else if (n_args == 3) {
				args.z += args_str[i];
			} else {
				return 0;
			}
		}
  }
  return n_args;
}

bool op_is_valid(const char op) {
	return (op == '+' || op == '-' || op == '*' || op == '/');
}

bool try_block_match(const char op1, const char op2,
														const std::string expr1, const std::string expr2) {
	return (op1 == op2 && expr1 == expr2);
}

// parse block of type {} or [], n is optional for {}
bool parse_block(const std::string &block, char &op, std::string &expr, int *n) {
	// get block type, progress index
	char end_bracket;
	if (block[0] == '{') {
		end_bracket = '}';
	} else if (block[0] == '[') {
		end_bracket = ']';
	} else {
		print_info("block invalid");
		return false;
	}

	// parse operator
	int field_start = 1;
	int field_end = block.find(',');

	if (field_end == std::string::npos) { return false; }
	std::string op_str = util::trim(block.substr(field_start, field_end - field_start));
	if (op_is_valid(op_str[0])) {
		op = op_str[0];
	} else {
		print_info("operator invalid");
		return false;
	}

	// parse expression and n if possible
	field_start = field_end + 1;
	field_end = block.find(',', field_start);

	if (field_end == std::string::npos) {
		field_end = block.find(end_bracket, field_start);
		if (field_end == std::string::npos) { 
			print_info("no end_bracket");
			return false; 
		}
		expr = block.substr(field_start, field_end - field_start);

	} else {
		expr = block.substr(field_start, field_end - field_start);
		field_start = field_end + 1;
		field_end = block.find(end_bracket, field_start);

		if (field_end == std::string::npos) { 
			print_info("no end_bracket");
			return false; 
		}
		std::string n_str = block.substr(field_start, field_end - field_start);
		if (n) {
			*n = std::atoi(n_str.c_str());
		}
	}

	return true;
}

// expects arg of form: base|pattern|{repeat}[scale]
// return false if parts are not valid, true if valid or incomplete
bool parse_arg(const std::string arg, std::string &base, std::string &pattern,
							 std::vector<std::string> &repeats, std::string & scale) {
	base.clear();
	pattern.clear();
	repeats.clear();
	scale.clear();
	
	// parse base
	if (int first_seperator = arg.find_first_of("|{["); first_seperator != std::string::npos) {
		base = arg.substr(0, first_seperator);
	} else {
		base = arg;
	}

	// parse scale
	int scale_open = 0;
	int scale_close = 0;
	if (scale_open = arg.find('['); scale_open != std::string::npos) {
		if (scale_close = arg.find(']'); scale_close != std::string::npos) {
			scale = arg.substr(scale_open, scale_close);
		} else {
			return false;
		}
	}

	// parse pattern
	int pattern_open = 0;
	int pattern_close = 0;
	if (pattern_open = arg.find('|'); pattern_open != std::string::npos) {
		if (pattern_close = arg.find('|', pattern_open + 1); pattern_close != std::string::npos) {
			pattern = arg.substr(pattern_open, pattern_close);
		} else {
			return false;
		}
	}

	// parse repeats_str
	std::string repeats_str;
	int repeats_start = 0;
	int repeats_end = 0;
	if (repeats_start = arg.find('{'); repeats_start != std::string::npos) {
		if (repeats_end = arg.find_last_of('}'); repeats_end != std::string::npos) {
			repeats_str = arg.substr(repeats_start, repeats_end - repeats_start + 1);

			// parse repeats_str into repeats vector
			bool expect_open = false;
			repeats.emplace_back();
			for (char c : repeats_str) {
				// validity check
				if (expect_open) {
					if (c == ' ') { continue; }
					if (c != '{') { 
						repeats.clear();
						return false; 
					}
					repeats.emplace_back();
					expect_open = false;
				}
				// append until '}'
				repeats.back() += c;
				if (c == '}') {
					expect_open = true;
				}
			}
		} else {
			return false;
		}
	}

	return true;
}

// arg can be empty
// error if rule_arg is empty, return early -> arg
std::string arg_rulearg_substitute(const std::string arg, const std::string rule_arg) {
	// handle incomplete
	if (rule_arg.empty()) {
		print_info("rule_arg incomplete");
		return arg;
	};

  std::string return_arg = "";
	// rule_arg is of type scaling
  if (rule_arg[0] == '[') {
		// append arg up until [ or end, then append rule_arg
    for (int i = 0; i < arg.size() && arg[i] != '['; i++) {
      return_arg += arg[i];
    }
    return_arg += rule_arg;
    return return_arg;

	// ---- rule_arg is of type repeat ----
  } else if (rule_arg[0] == '{') {

    // parse arg of form base{repeat}{repeat}...[scale]
    // later condense to base|{repeat}{repeat}|8[scale]
    std::string base = "";
    std::string pattern = "";
    std::vector<std::string> repeats{};
    std::string scale = "";
		if (!arg.empty()) {
			if (!parse_arg(arg, base, pattern, repeats, scale)) {
				print_info("arg parse error");
				return "";
			}
		}

		// parse rule_repeat
    char rule_repeat_op;
    std::string rule_repeat_expr;
    if (!parse_block(rule_arg, rule_repeat_op, rule_repeat_expr, nullptr)) {
			print_info("repeat parse error");
      return arg;
    }

		// ---- arg has no repeat blog ----
		if (repeats.size() == 0) {
			std::string new_rule_repeat = fmt::format("{{{},{},{}}}", rule_repeat_op, rule_repeat_expr, 1);
			return base + pattern + new_rule_repeat + scale;
		// ---- arg has a repeat block ----
		} else if (repeats.size() == 1) {
			// parse arg_repeat
			char repeat_op = '\0';
			std::string repeat_expr = "";
			int repeat_n = 1; // default to 1 if no repeat_n specified
			if (!parse_block(repeats.back(), repeat_op, repeat_expr, &repeat_n)) {
				print_info("repeat parse error");
				return "";
			}
			if(try_block_match(repeat_op, rule_repeat_op, repeat_expr, rule_repeat_expr)) {
				std::string changed_repeat = fmt::format("{{{},{},{}}}", repeat_op, repeat_expr, repeat_n + 1);
				return base + pattern + changed_repeat + scale;
			} else {
				std::string new_repeat = fmt::format("{{{},{},{}}}", repeat_op, repeat_expr, repeat_n);
				std::string new_rule_repeat = fmt::format("{{{},{},{}}}", rule_repeat_op, rule_repeat_expr, 1);
				return base + pattern + new_repeat + new_rule_repeat + scale;
				// try_condense_pattern();
			}
		// ---- arg has multiple repeat blocks ----
		} else if (repeats.size() > 1){

			// check affine
			bool is_affine = true;
			std::vector<char> ops{};
			for (std::string repeat: repeats) {
				char repeat_op;
				std::string repeat_expr;
				if (!parse_block(repeat, repeat_op, repeat_expr, nullptr)) {
					print_info("repeat parse error");
					return "";
				}
				ops.push_back(repeat_op);
			}
			bool op_is_muldiv = (ops.back() == '*' || ops.back() == '/');
			for (char op : ops) {
				if (((op == '*' || op == '/') && !op_is_muldiv) ||
						((op == '+' || op == '-') && op_is_muldiv)) {
					is_affine = false;
					break;
				}
			}

			fmt::print("[DEBUG] affine = {}\n", is_affine);

			if (is_affine) {
				bool match = false;
				for (int i = 0; i < repeats.size(); i++) {
					char repeat_op;
					std::string repeat_expr;
					int repeat_n = 1; // default to 1 if no repeat_n specified
					if (!parse_block(repeats[i], repeat_op, repeat_expr, &repeat_n)) {
						print_info("repeat parse error");
						return "";
					}

					if(try_block_match(repeat_op, rule_repeat_op, repeat_expr, rule_repeat_expr)) {
						match = true;
						std::string changed_repeat = fmt::format("{{{},{},{}}}", repeat_op, repeat_expr, repeat_n + 1);
						repeats[i] = changed_repeat;
						std::string repeats_str = "";
						for (std::string repeat : repeats) {
							repeats_str += repeat;
						}
						return base + pattern + repeats_str + scale;
					}
				}
				if (!match) {
					std::string repeats_str = "";
					for (std::string repeat : repeats) {
						repeats_str += repeat;
					}
					std::string new_repeat = fmt::format("{{{},{},{}}}", rule_repeat_op, rule_repeat_expr, 1);
					return base + pattern + repeats_str + new_repeat + scale;
					// try_condense_pattern();
				}
			} else if (!is_affine) {
				char repeat_op;
				std::string repeat_expr;
				int repeat_n;
				if (!parse_block(repeats.back(), repeat_op, repeat_expr, &repeat_n)) {
					print_info("repeat parse error");
					return "";
				}

				if(try_block_match(repeat_op, rule_repeat_op, repeat_expr, rule_repeat_expr)) {
					std::string changed_repeat = fmt::format("{{{},{},{}}}", repeat_op, repeat_expr, repeat_n + 1);
					repeats.back() = changed_repeat;
					std::string repeats_str = "";
					for (std::string repeat : repeats) {
						repeats_str += repeat;
					}
					return base + pattern + repeats_str + scale;
				} else {
					std::string repeats_str = "";
					for (std::string repeat : repeats) {
						repeats_str += repeat;
					}
					std::string new_repeat = fmt::format("{{{},{},{}}}", rule_repeat_op, rule_repeat_expr, 1);
					return base + pattern + repeats_str + new_repeat + scale;
					// try_condense_pattern();
				}
			}
		}
  } else { // rule_arg is of type no-ref
    return rule_arg;
  }
	return "";
}

// try to match symbol with a rule
// if match is found, replace symbol with rule
// returns:
// 1. rule substituted for symbol if rule applies
// 2. symbol<args> if no rule applies
// 3. symbol if error
std::string _maybe_apply_rule(Module &module, const char symbol, const std::string args_str) {
	std::string default_return = "";
	if (args_str.empty()) {
		default_return = fmt::format("{}", symbol);
	} else {
		default_return = fmt::format("{}<{}>", symbol, args_str);
	}

	// auto result = get_default(symbol);
	// if (result == std::nullopt) { return ""; }
	// float x_default = result.value();

	// ---- extract args of symbol ----
	std::string x = "";
	std::string y = "";
	std::string z = "";

	Args args{};

 	int n_args = 1;
	if (!args_str.empty()) {
 		n_args = parse_args(args_str, args);
		if (n_args == 0) { 
			print_info("parse_args error");
			return fmt::format("{}", symbol); // DONO
		}
	}

	// fmt::print("args: {}\n", args);

	// ---- try to match a rule ----
	for (auto &rule : module.lstring_spec.rules) {
		// Matching phase:
		// 2. test if there is a matching symbol
		// -> 3. then test if the number of args matches
		// -> 4. then if there is a "internal" condition, eval_args()
		// -> if the condition is external, means does not refer to x,y,z dont eval

		// check symbol, check args present, check args condition
		// -> implement later... check context, check environmental conditions
		// why isnt this just a char array?
		std::string condition = rule.textfield_condition;
		std::string text = rule.textfield_rule;


		// print_info("check_1");

		if (rule.symbol != symbol) { continue; }
		// if (rule.n_args != n_args) { continue; }

		// TODO: check against condition
		if (!condition.empty()) {
			std::array<float, 3> args_ary = symbol_eval_args(module, symbol, args_str);
			auto result = _eval_expr(module, condition, &args_ary[0], &args_ary[1],
					&args_ary[2]);
			if (!result) {
				// TODO: HALT_GENERATION
				rule.condition_state = util::STATE::ERROR;
				continue;
			} else {
				if (!util::equal_epsilon(result.value(), 1.0)) {
					continue;
				}
			}
		}

		// ---- rule matched -> replace symbol with rule ----
		std::string return_str = "";
		int index = 0;
		while (index < (int)text.size()) {
			// append args_field if text[index] is not symbol
			if (text[index] == '<') {
				while (text[index] != '>') {
					if (index >= (int)text.size()) {
						print_info("rule block incomplete");
						return default_return; 
					}
					return_str += text[index++];
				}
				return_str += text[index++];

			// append all other characters
			} else if (text[index] != symbol ) {
				return_str += text[index++];
			// substitute args for rule_args
			} else if (text[index] == symbol) {
				// text[index] == symbol
				return_str += text[index++]; // append symbol
				if (index >= text.size()) { continue; }
				if (text[index] != '<') { continue; } // replace arg with default

				// ---- symbol with args found in rule ----
				index++;
				std::string rule_symbol_args = util::get_substr(text, index, '>');
				if (rule_symbol_args.empty()) {
					print_info("rule block incomplete");
					return default_return; 
				}
				index += rule_symbol_args.size() + 1; // move index after '>'

				// the args of the symbol occurence in rule
				Args rule_args{};
				parse_args(rule_symbol_args, rule_args);

				// what to do if impty?
				std::string fin_x = arg_rulearg_substitute(x, rule_args.x); // arg can be empty
				return_str += '<';
				return_str += fin_x;
				if (n_args > 1) {
					std::string fin_y = arg_rulearg_substitute(y, rule_args.y);
					return_str += fmt::format(";{}", fin_y);
				}
				if (n_args > 2) {
					std::string fin_z = arg_rulearg_substitute(z, rule_args.z);
					return_str += fmt::format(";{}", fin_z);
				}
				return_str += '>';
			}
		} // end while()
		return return_str;
	} // end for()

	return default_return; 
}

State generate_lstring(Module &module) {
	for (int i = 0; i < module.lstring_spec.iterations; i++) {
		State s = expand_lstring(module);
		if (s == State::Error) { return s; }
	}
	return State::True;
}

State expand_lstring(Module &module) {
	auto &current_iteration = module.lstring_spec.current_iteration;

	std::string &lstring = module.lstring;
	if (!module.generation_started) {
		lstring.assign(module.lstring_spec.axiom);
	}
  std::string lstring_expanded = "";

	// expand lstring and save into lstring_expanded
	int index = 0;
	while (index < lstring.size()) {
		char c = lstring[index];
		// some symbols can just be copied directly
		if (c == '[' || c == ']') {
			lstring_expanded += lstring[index++];
		} else {
			if (index + 1 >= lstring.size()) { // this makes while condition irrelevant
				lstring_expanded += _maybe_apply_rule(module, lstring[index], "");
				break;
			} else if (lstring[index + 1] != '<') {
				lstring_expanded += _maybe_apply_rule(module, lstring[index++], "");
			} else {
				index += 2; // move to after '<'
				std::string args = util::get_substr(lstring, index, '>');
				if (args.empty()) {
					print_info("lstring invalid, expand failed");
					return State::False;
				}
				lstring_expanded += _maybe_apply_rule(module, c, args);
				index += args.size() + 1; // move to after '>'
			}
		}
	}
	// replace lstring with lstring_expanded and return
	current_iteration++;
	module.lstring = lstring_expanded;
	return State::True;
}


// TODO serialize lsystem
// bool save_rule_as_file(System::Rule &rule, const std::string &save_file_name) {
// 	std::string save_file_full_path = app::context.save_path + '/' + save_file_name;
//
// 	fmt::print("save file: {}\n", save_file_full_path);
//
// 	FILE* fp = std::fopen(save_file_full_path.c_str(), "w");
// 	if (!fp) {
// 		std::puts("std::fopen failed");
// 		return false;
// 	}
//
// 	bool is_ok = true;
// 	constexpr int n_items = app::gui.textfield_size;
//   if (fwrite(rule.condition, sizeof(rule.condition[0]), n_items, fp) != n_items) {
// 		is_ok = false;
// 	}
// 	if (fwrite(rule.text, sizeof(rule.text[0]), n_items, fp) != n_items) {
// 		is_ok = false;
// 	}
//
// 	fclose(fp);
//
// 	// delete file if frwite() failed
// 	if (!is_ok) {
// 		assert(std::remove(save_file_full_path.c_str()) == 0);
// 	}
// 	return true;
// }


// TODO load lsystem from file
// bool load_rule_from_file(System::Rule &rule, std::string &save_file_name) {
// 	std::string save_file = app::context.save_path + '/' + save_file_name;
//
// 	FILE* fp = std::fopen(save_file.c_str(), "r");
// 	if (!fp) {
// 		std::puts("std::fopen failed");
// 		return false;
// 	}
//
// 	constexpr int n_items = app::gui.textfield_size;
// 	char temp_buf[n_items];
//   if (std::fread(temp_buf, sizeof(temp_buf[0]), n_items, fp) == n_items) {
// 		std::memcpy(rule.condition, temp_buf, n_items * sizeof(temp_buf[0]));
// 	} else {
// 		std::fclose(fp);
// 		return false;
// 	}
//   if (std::fread(temp_buf, sizeof(temp_buf[0]), n_items, fp) == n_items) {
// 		std::memcpy(rule.text, temp_buf, n_items * sizeof(temp_buf[0]));
// 	} else {
// 		std::fclose(fp);
// 		return false;
// }
//
// 	std::fclose(fp);
// 	return true;
// }


// scan the save files
// std::optional<std::vector<std::string>> scan_saves() {
// 	namespace fs = std::filesystem;
// 	std::string save_path = app::context.save_path;
//
// 	std::vector<std::string> names;
// 	if (fs::exists(save_path) && fs::is_directory(save_path)) {
// 			for (auto const& entry : fs::directory_iterator(save_path)) {
// 					if (entry.is_regular_file()) {
// 							names.push_back(entry.path().filename().string());
// 					}
// 			}
// 	}
// 	std::sort(names.begin(), names.end());
// 	if (names.size() > 0) {
// 		return names;
// 	} else {
// 		return {};
// 	}
// }
} // namespace ls
