#include "lsystem_new.hpp"

namespace lsystem_new {
State _expand_lstring(Module* module);
State expand_lstring(Module* module);
State regenerate_lstring(Module* module);

std::optional<double>
evaluate_expression(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    std::string& expression_string);

std::optional<std::string>
evaluate_production(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& production);

// this is no longer needed, this will now return a vector<double>
int parse_args(const std::string &args_string, ArgsString &args) {
	if (args_string.empty()) { return 0; }
  int n_args = 1;
  for (int i = 0; i < args_string.size(); i++) {
    if (args_string[i] == ';') {
      n_args++;
    } else {
			if (n_args == 1) {
				args.x += args_string[i];
			} else if (n_args == 2) {
				args.y += args_string[i];
			} else if (n_args == 3) {
				args.z += args_string[i];
			} else {
				return 0;
			}
		}
  }
  return n_args;
}

ArgsValue symbol_get_defaults(Module& module, const char symbol) {
	auto symbol_category = get_symbol_category(symbol);
	if (symbol_category == std::nullopt) { return { 0.0, 0.0, 0.0 }; }
	Var x_default = module.get_default_var(symbol_category.value());
	return { x_default.value, 0.0, 0.0 };
}

ArgsValue args_string_get_values(Module& module, const char symbol,
																 const std::string args_string) {
	ArgsValue default_args = symbol_get_defaults(module, symbol);
	ArgsString string_args{};
	parse_args(args_string, string_args);
	ArgsValue args{};
	double x = std::atof(string_args.x.c_str());
	args.x = (x == 0.0) ? default_args.x : x;
	double y = std::atof(string_args.y.c_str());
	args.y = (y == 0.0) ? default_args.y : y;
	double z = std::atof(string_args.z.c_str());
	args.z = (z == 0.0) ? default_args.z : z;
	return args;
}

// 1. evaluate axiom

// args_string are 3 values seperated by semicolon
std::string maybe_apply_rule(Module &module, const char symbol, const std::string args_string) {
	std::string default_return = "";
	if (args_string.empty()) {
		default_return = fmt::format("{}", symbol);
	} else {
		default_return = fmt::format("{}{{}}", symbol, args_string);
	}

	// auto result = get_default(symbol);
	// if (result == std::nullopt) { return ""; }
	// float x_default = result.value();
	//
	// A -> A{x'*5; sin(z); 3}

	// ---- extract args of symbol ----
	ArgsString args{};
 	int n_args = 1;
	if (!args_string.empty()) {
 		n_args = parse_args(args_string, args);
		if (n_args == 0) { 
			print_info("parse_args error");
			return fmt::format("{}", symbol); // DONO
		}
	}

	// fmt::print("args: {}\n", args);

	// ---- try to match a rule ----
	for (auto &rule : module.lstring_spec.rules) {
		std::string condition = rule.textfield_condition;
		std::string text = rule.textfield_rule;

		if (rule.symbol != symbol) { continue; }

		if (!condition.empty()) {
			ArgsValue args_ary = symbol_eval_args(module, symbol, args_str);
			auto result = module.evaluate_expression(condition, &args_ary[0], &args_ary[1],
					&args_ary[2], true);
			fmt::print("condition: {}\n", result.value_or(-1));
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
				ArgsString rule_args{};
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

State maybe_regenerate_modules(LsystemManager& lsystem_manager) {
	for (auto &[key, module] : lsystem_manager.modules) {
		auto module_ptr = module.get();
		if (module_ptr->generation_manager.regen_needed == false) { continue; }
		module_ptr->lstring.clear();
		std::string axiom = module_ptr->generation_manager.axiom;
		std::unordered_map<std::string, double> local_variables{};
		auto result = evaluate_production(local_variables, module_ptr->global_variables, axiom);
		if (result == std::nullopt) {
			// TODO: instead of printing, draw text in red
			print_info("Axiom could not be evaluated\n");
			continue;
		}
		module_ptr->lstring = result.value();
		// TODO tomorrow
		// Expand once by default, then generate untill iteration_count reached
		{
			State state = expand_lstring(module_ptr);
		}
		{
			State state = regenerate_lstring(module_ptr);
		}
	}
	return State::True;
}

// extra wrapper for quantized
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
	for (auto& variable : local_variables) {
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
			for (int i = 0; i < args.size(); i++) {
				if (i + 1 < args.size()) {
					evaluated_production += fmt::format("{{}};", result.value());
				} else {
					evaluated_production += fmt::format("{{}}", result.value());
				}
			}
			index++;
		} else {
			evaluated_production += production[index++];
		}
	}
}

State regenerate_lstring(Module* module) {
	for (int i = 0; i <= module->generation_manager.iteration_count; i++) {
		State state = _expand_lstring(module);
		if (state == State::Error || state == State::False) { return state; }
	}
	return State::True;
}

State expand_lstring(Module* module) {
	State state = _expand_lstring(module);
	if (state == State::Error || state == State::False) { return state; }
	module->generation_manager.iteration_count++;
	return State::True;
}

State _expand_lstring(Module* module) {
	int& iteration_count = module->generation_manager.iteration_count;
	std::string axiom = module->generation_manager.axiom;

	const std::string& string_to_expand = (iteration_count == 0) ?
		axiom : module->lstring;
	std::string& lstring_buffer = module->generation_manager.lstring_buffer;

	int index = module->generation_manager.current_index;
	while (index < string_to_expand.size()) {
		// ---- check time -> stop expansion  ----
		util::ms elapsed = util::Clock::now() - app::context.frame_start;
		if ((elapsed / app::context.frame_time) >= 0.6) {
			module->generation_manager.current_index = index;
			return State::False;
		}
		// ---- expand one symbol ----
		char c = string_to_expand[index];
		if (c == '[' || c == ']') {
			lstring_buffer  += string_to_expand[index++];
		} else {
			if (index + 1 >= string_to_expand.size()) { // this makes while condition irrelevant
				lstring_buffer  += maybe_apply_rule(*module, string_to_expand[index], "");
				break;
			} else if (string_to_expand[index + 1] != '<') {
				lstring_buffer  += maybe_apply_rule(*module, string_to_expand[index++], "");
			} else {
				index += 2; // move to after '<'
				std::string args = util::get_substr(string_to_expand, index, '>');
				if (args.empty()) {
					print_info("string_to_expand invalid, expand failed");
					return State::False;
				}
				lstring_buffer  += maybe_apply_rule(module, symbol, args);
				index += args.size() + 1; // move to after '>'
			}
		}
	}

	// ---- expansion completed ----
	module->generation_manager.current_index = 0;
	module->lstring = lstring_buffer;
	lstring_buffer.clear();
	return State::True;
}
} // lsystem_new
