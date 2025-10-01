#include "lsystem_new.hpp"

namespace lsystem_new {
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
    print_info("Could not evaluate expression");
    fmt::print("expression: {}\n", expression_string);
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
			std::string arg_block = get_arg_block(production, index);
			if (arg_block.empty()) { 
				print_info("Arg block empty");
				return std::nullopt; 
			}
			index += arg_block.size();

			std::vector<double> args =
				evaluate_arg_block(local_variables, global_variables, arg_block);
			if (args.empty()) { 
				print_info("Args could not be parsed");
				return std::nullopt;
			}

			evaluated_production += '{';
			for (int i = 0; i < args.size(); i++) {
				if (i + 1 < args.size()) {
					evaluated_production += fmt::format("{};", args[i]);
				} else {
					evaluated_production += fmt::format("{}", args[i]);
				}
			}
			evaluated_production += '}';

		} else {
			evaluated_production += production[index++];
		}
	}
}

std::vector<std::string> split_arg_block(const std::string &arg_block) {
	if (arg_block.empty()) { return std::vector<std::string>{}; }
	if (arg_block.front() != '{') { return std::vector<std::string>{}; }
	if (arg_block.back() != '}') { return std::vector<std::string>{}; }
	std::vector<std::string> args{};
	std::string current_string{};
  for (int i = 1; i < arg_block.size() - 1; i++) {
    if (arg_block[i] == ';') {
			args.push_back(current_string);
			current_string.clear();
    } else {
			current_string += arg_block[i];
		}
  }
	args.push_back(current_string);
  return args;
}

std::string get_arg_block(const std::string lstring, const int index) {
	if (lstring.empty() || index >= lstring.size())  { return std::string{}; }
	std::string arg_block{};
	if (lstring[index] != '{') { return std::string{}; }
	int new_index = index;
	for (; lstring[new_index] != '}'; new_index++) {
		if (new_index >= lstring.size()) { return std::string{}; }
		arg_block += lstring[new_index];
	}
	arg_block += lstring[new_index];
	return arg_block;
}

std::vector<double> parse_arg_block(const std::string arg_block) {
	std::vector<double> args{};
	std::vector<std::string> string_args = split_arg_block(arg_block);
	if (string_args.empty()) { return std::vector<double>{}; }

	// if it fails it just fills 0, should do something else
	for (int i = 0; i < string_args.size(); i++) {
		char* end = nullptr;
		double value = std::strtod(string_args[i].c_str(), &end);
		if (end == string_args[i].c_str() || *end != '\0') {
			print_info("could not parse arg");
			return std::vector<double>{};
		}
		args.push_back(value);
	}
	return args;
}

std::vector<double>
evaluate_arg_block(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& arg_block) {
	std::vector<double> args{};
	std::vector<std::string> string_args = split_arg_block(arg_block);
	if (string_args.empty()) { return std::vector<double>{}; }
	for (int i = 0; i < string_args.size(); i++) {
		auto result = evaluate_expression(local_variables, global_variables, string_args[i]);
		if (result == std::nullopt) {
			print_info("could not evaluate arg");
			return std::vector<double>{}; 
		}
		args.push_back(result.value());
	}
	return args;
}

std::optional<int> 
parse_symbol(const std::string lstring, const int index, char& symbol,
             std::vector<double> &args) {
	if (lstring.empty() || index >= lstring.size())  { return std::nullopt; }

	int local_index = index;
	symbol = lstring[local_index++];
	if (local_index >= lstring.size()) { return 0; }
	if (lstring[local_index] != '{')  { return local_index; }

	std::string arg_block = get_arg_block(lstring, local_index);
	if (arg_block.empty()) { 
		print_info("Arg block empty");
		return std::nullopt; 
	}

	local_index += arg_block.size();
	args = parse_arg_block(arg_block);
	if (args.empty()) { 
		print_info("Args could not be parsed");
		return std::nullopt;
	}
	
	return local_index;
}

std::unordered_map<std::string, double> args_to_map(std::vector<double> args) {
	std::unordered_map<std::string, double> args_map{};
	if (args.size() > 0) args_map["x"] = args[0];
	if (args.size() > 1) args_map["y"] = args[1];
	if (args.size() > 2) args_map["z"] = args[2];
	return args_map;
}

std::string maybe_apply_rule(Generator* generator, const char symbol, std::vector<double> args) {
	std::unordered_map<std::string, double> local_variables = args_to_map(args);

	// ---- try to match a rule ----
	for (auto &production : generator->productions) {
		std::string condition = production.condition;
		std::string rule = production.rule;

		if (production.symbol != symbol) { continue; }

		if (!condition.empty()) {
			auto result = evaluate_expression(local_variables, generator->global_variables, condition);
			if (!result) {
			// TODO: draw rule text in red
				print_info("");
				continue;
			}
			if (result.value() == false) {
				continue;
			}
		}

		// ---- rule matched -> replace symbol with rule ----
		auto result = evaluate_production(local_variables, generator->global_variables, rule);
		if (result == std::nullopt) { return fmt::format("{}", symbol); }
		return result.value();
	}
}

State reset_generator(Generator* generator) {
	generator->lstring.clear();
	generator->current_iteration = 0;
	std::string axiom = generator->axiom;
	std::unordered_map<std::string, double> local_variables{};
	auto result = evaluate_production(local_variables, generator->global_variables, axiom);
	if (result == std::nullopt) {
		// TODO: draw gui text red
		print_info("Axiom could not be evaluated\n");
		return State::False;
	}
	generator->lstring = result.value();
	generator->current_iteration++;
	return State::True;
}

State generate_timed(Generator* generator) {
	auto& iterations = generator->iterations;
	auto& current_iteration = generator->current_iteration;
	for (int i = current_iteration; i < iterations; i++) {
		State state = _expand_lstring(generator);
		if (state == State::Error || state == State::False) { return state; }
		current_iteration++;
	}
	return State::True;
}


State _expand_lstring(Generator* generator) {
	int index = generator->current_index;
  while (index < generator->lstring.size()) {

    // ---- check time ----
    util::ms elapsed = util::Clock::now() - app::context.frame_start;
    if ((elapsed / app::context.frame_time) >= 0.6) {
      generator->current_index = index;
      return State::False;
    }

		char symbol{};
		std::vector<double> args{};
		auto result = parse_symbol(generator->lstring, index, symbol, args);
		if (result == std::nullopt) {
			return State::Error; 
		}
		if (result.value() == 0) { // 0 means last char
      maybe_apply_rule(generator, symbol, std::vector<double>{});
			break;
		}
    maybe_apply_rule(generator, symbol, args);
		index = result.value();
  }

  // ---- expansion completed ----
  generator->current_index = 0;
  generator->lstring = generator->lstring_buffer;
  generator->lstring_buffer.clear();
  return State::True;
}

State update_generator(Generator* generator) {

	// if reset is needed, reset, then return, not timed,  never takes that long
	if (generator->reset_needed == true) {
		generator->reset_needed = false;
		State state = reset_generator(generator);
		return state;
	}

	{
		// gets most of frametime
		State state = generate_timed(generator);
		if (state == State::Error || state == State::False) { return state; }
	}

	return State::True;
}
} // namespace lsystem_new
