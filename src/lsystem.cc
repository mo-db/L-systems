#include "lsystem.hpp"

namespace lsystem {
// State Module::update_vars() {
//   for (int i = global_vars.size() - 1; i >= 0; i--) {
//     Var &var = global_vars[i];
//     if (*(var.expr) == '\0') {
//       continue;
//     }
//     if (var.use_slider) {
//     } else {
//       std::string glob_var_str = var.expr;
//       var.value = evaluate_expression(
// 				glob_var_str, nullptr, nullptr, nullptr, false).value_or(0.f);
//     }
//   }
//   return State::True;
// }

std::optional<SymbolCategory> get_symbol_category(const char ch) {
	if (std::isalpha(ch)) {
		return SymbolCategory::Move;
	}
	if (ch == '-' || ch == '+') {
		return SymbolCategory::Rotate;
	}
	if (ch == '^' || ch == '&') {
		return SymbolCategory::Width;
	}
	return std::nullopt;
}

char get_symbol(SymbolIdentifier symbol_identifier) {
  return symbols[static_cast<std::size_t>(symbol_identifier)];
}

State update_module(Module* module, draw::FrameBuf& fb) {
	auto& generation_manager = module->generation_manager;

	// if reset is needed, reset, then return, not timed,  never takes that long
	if (generation_manager.reset_needed == true) {
		generation_manager.reset_needed = false;
		State state = reset_lstring(module);
		return state;
	}

	{
		// gets most of frametime
		State state = generate_lstring_timed(module);
		if (state == State::Error || state == State::False) { return state; }
	}

	// draw and gen the plant only if generation of the lstring is finished
	bool plant_generation_finished = false;
	bool plant_drawing_finished = false;
	{
		// gets some percentage of frametime
		State state = generate_plant_timed(module);
		if (state == State::Error) { return state; }
		if (state == State::True) { plant_generation_finished = true; }
	}
	{
		// gets some percentage of frametime
		State state = draw_plant_timed(module, fb);
		if (state == State::Error) { return state; }
		if (state == State::True) { plant_drawing_finished = true; }
	}

	// return true if the module is fully updated, all work finished
	return (plant_generation_finished && plant_drawing_finished) ?
		State::True : State::False;
}

Vec2 _calculate_move(Plant &plant, const float length) {
	Turtle &turtle = plant.turtle;
	Vec2 position = plant.nodes[turtle.node_id];
	position.x += length * cos(turtle.angle);
	position.y += length * -sin(turtle.angle);
	return position;
}
void _turn(Turtle &turtle, const float angle) { turtle.angle += angle; }

State _turtle_action(Plant& plant, const char symbol, double x) {
	Turtle &turtle = plant.turtle;
	std::vector<Turtle> &turtle_stack = plant.turtle_stack;

	auto symbol_category = get_symbol_category(symbol);
	if (symbol_category == std::nullopt) { return State::False; }

	// TODO: visable not used
	if (symbol_category.value() == SymbolCategory::Move) {
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
	return State::True;
}

// this is no longer needed, this will now return a vector<double>
std::vector<std::string> split_args(const std::string &args_string) {
	if (args_string.empty()) { return std::vector<std::string>{}; }
	std::vector<std::string> args{};
	std::string current_string{};
  for (int i = 0; i < args_string.size(); i++) {
    if (args_string[i] == ';') {
			args.push_back(current_string);
			current_string.clear();
    } else {
			current_string += args_string[i];
		}
  }
	args.push_back(current_string);
  return args;
}

std::vector<double> get_args_values(const std::string args_string) {
	std::vector<double> args{};
	std::vector<std::string> split_args_string = split_args(args_string);
	for (int i = 0; i < split_args_string.size(); i++) {
		args.push_back(std::atof(split_args_string[i].c_str()));
	}
	return args;
}

State generate_plant_timed(Module* module){
	std::string &lstring = module->lstring;
	Plant &plant = module->plant;
	int &current_index = plant.current_lstring_index;

	int index = current_index;
	while (index < lstring.size()) {
		// ---- timing ----
		util::ms elapsed = util::Clock::now() - app::context.frame_start;
		if ((elapsed / app::context.frame_time) >= 0.6) {
			current_index = index;
			return State::False;
		}
			
		auto symbol_category = get_symbol_category(lstring[index]);
		if (symbol_category == std::nullopt) { return State::Error; }
		double symbol_default = module->symbol_defaults[symbol_category.value()];

		if (index + 1 >= lstring.size() || lstring[index + 1] != '{') { 
			State state  = _turtle_action(plant, lstring[index], symbol_default);
		} else if (lstring[index + 1] == '{') {
			index += 2; // move to after '{'
			std::string args_string = util::get_substr(lstring, index, '}');
			if (args_string.empty()) { return State::Error; }
			std::vector<double> args_values = get_args_values(args_string);
			State state  = _turtle_action(plant, lstring[index], args_values[0]);
			index += args_string.size() + 1; // move to after '}'
		}
		index++;
	}

	current_index = 0;
	return State::True;
}

State draw_plant_timed(Module* module, draw::FrameBuf &fb) {
	auto &plant = module->plant;
	for (int i = plant.current_branch; i < plant.branches.size(); i++) {
		// ---- timing ----
		util::ms elapsed = util::Clock::now() - app::context.frame_start;
		// print_info(fmt::format("draw_plant elapsed: {}\n",
		// 			(elapsed / app::context.frame_time)));
		if ((elapsed / app::context.frame_time) >= 0.9) {
			plant.current_branch = i;
			return State::False;
		}

		auto &branch = plant.branches[i];
		const Vec2 &node1 = plant.nodes[branch.node1_id];
		const Vec2 &node2 = plant.nodes[branch.node2_id];
		draw::wide_line(fb, Line2{node1, node2}, plant.color, branch.wd);
	}
	plant.current_branch = 0;
	return State::True;
}

std::string maybe_apply_rule(Module* module, const char symbol, std::vector<double> args) {
	// CHANGE:
	std::unordered_map<std::string, double> local_variables { {"x", args[0]}, {"y", args[1]}, {"z", args[2]} };

	// ---- try to match a rule ----
	for (auto &rule : module->generation_manager.rules) {
		std::string condition = rule.textfield_condition;
		std::string text = rule.textfield_rule;

		if (rule.symbol != symbol) { continue; }

		if (!condition.empty()) {
			auto result = evaluate_expression(local_variables, module->global_variables, text);
			// TODO: draw rule text in red
			if (!result) {
				print_info("fuck what to do with condition wrong\n");
				continue;
			}
			if (!util::equal_epsilon(result.value(), 1.0)) {
				continue;
			}
		}

		// ---- rule matched -> replace symbol with rule ----
		auto result = evaluate_production(local_variables, module->global_variables, text);
		if (result == std::nullopt) { return fmt::format("{}", symbol); }
		return result.value();
	}
}

State reset_lstring(Module* module) {
	module->lstring.clear();
	module->generation_manager.current_iteration = 0;
	std::string axiom = module->generation_manager.axiom;
	std::unordered_map<std::string, double> local_variables{};
	auto result = evaluate_production(local_variables, module->global_variables, axiom);
	if (result == std::nullopt) {
		// TODO: instead of printing, draw text in red
		print_info("Axiom could not be evaluated\n");
		return State::False;
	}
	module->lstring = result.value();
	module->generation_manager.current_iteration++;
	return State::True;
}

// implement extra wrapper for quantized
// double quantize_variable(double value, double epsilon) {
//   return std::round(value / epsilon) * epsilon;
// }
//
// std::unordered_map<std::string, double>
// quantize_map(const std::unordered_map<std::string, float>& input,
//              double epsilon)
// {
//     std::unordered_map<std::string, double> result;
//     result.reserve(input.size());
//
//     for (const auto& [label, value] : input) {
//         result[label] = quantize_variable(value, epsilon);
//     }
//
//     return result;
// }

// CHANGE: dont pass variables by reference???
// 1. have global symbol storage per module -> global vars, constants like pi, e etc...
// 2. pass local variables, per value not per reference
std::optional<double>
evaluate_expression(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& expression_string) {
	typedef double T;
  typedef exprtk::symbol_table<T> symbol_table_t;
  typedef exprtk::expression<T> expression_t;
  typedef exprtk::parser<T> parser_t;
  symbol_table_t symbol_table;

	for (auto& variable : local_variables) {
    symbol_table.add_variable(variable.first, variable.second);
  }
	for (auto& variable : global_variables) {
    symbol_table.add_variable(variable.first, variable.second);
  }

  // parse expression
  expression_t expression;
  expression.register_symbol_table(symbol_table);
  parser_t parser;
  if (!parser.compile(expression_string, expression)) {
    fmt::print("Could not evaluate expression:\n");
    fmt::print("expr_string: {}\n", expression_string);
    return {};
  }
  return (T)expression.value();
}

// return nullopt if eval failed
// CHANGE: maybe make a function: parse_braced_args()
std::optional<std::string>
evaluate_production(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& production) {
	std::string evaluated_production{};
	std::string expression{};
	int index = 0;
	while (index < production.size()) {
		if (production[index] == '{') {
			std::vector<double> args{};
			expression.clear();
			while (production[index] != '}') {
				if (index >= production.size()) { return evaluated_production; }
				if (production[index] == ';') {
					auto result =
						evaluate_expression(local_variables, global_variables, expression);
					if (result == std::nullopt) { return std::nullopt; }
					args.push_back(result.value());
					expression.clear();
				}
				expression += production[index++];
			}
			auto result =
				evaluate_expression(local_variables, global_variables, expression);
			if (result == std::nullopt) { return std::nullopt; }
			evaluated_production += '{';
			for (int i = 0; i < args.size(); i++) {
				if (i + 1 < args.size()) {
					evaluated_production += fmt::format("{};", result.value());
				} else {
					evaluated_production += fmt::format("{}", result.value());
				}
			}
			evaluated_production += '}';
			index++;
		} else {
			evaluated_production += production[index++];
		}
	}
}

State generate_lstring_timed(Module* module) {
	auto& iterations = module->generation_manager.iterations;
	auto& current_iteration = module->generation_manager.current_iteration;
	for (int i = current_iteration; i < iterations; i++) {
		State state = _expand_lstring(module);
		if (state == State::Error || state == State::False) { return state; }
		current_iteration++;
	}
	return State::True;
}

State _expand_lstring(Module* module) {
	const std::string& lstring = module->lstring;
	std::string& lstring_buffer = module->generation_manager.lstring_buffer;
	int index = module->generation_manager.current_index;

	while (index < lstring.size()) {
		// ---- check time -> stop expansion  ----
		util::ms elapsed = util::Clock::now() - app::context.frame_start;
		if ((elapsed / app::context.frame_time) >= 0.6) {
			module->generation_manager.current_index = index;
			return State::False;
		}

		// ---- expand one symbol ----
		if (lstring[index] == '[' || lstring[index] == ']') {
			lstring_buffer  += lstring[index++];
		} else if (index + 1 >= lstring.size() || lstring[index + 1] != '{') { 
			lstring_buffer  += maybe_apply_rule(module, lstring[index], std::vector<double>{});
		} else if (lstring[index + 1] == '}') {
			index += 2; // move to after '{'
			std::string args_string = util::get_substr(lstring, index, '}');
			if (args_string.empty()) { return State::Error; }
			std::vector<double> args_values = get_args_values(args_string);
			lstring_buffer  += maybe_apply_rule(module, lstring[index], args_values);
			index += args_string.size() + 1; // move to after '}'
		}
		index++;
	}

	// ---- expansion completed ----
	module->generation_manager.current_index = 0;
	module->lstring = lstring_buffer;
	lstring_buffer.clear();
	return State::True;
}
} // lsystem_new
