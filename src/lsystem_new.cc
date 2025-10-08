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
	if (ch == '[' || ch == ']') {
		return SymbolCategory::Stack;
	}

	LOG_ERROR(app::context.logger, "symobl has no category: {}", ch);
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
std::expected<double, Error>
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
		return std::unexpected(Error::Syntax);
  }
  return (T)expression.value();
}

// return nullopt if eval failed
// CHANGE: maybe make a function: parse_braced_args()
std::expected<std::string, Error>
evaluate_production(std::unordered_map<SymbolCategory, double>& symbol_defaults,
                    std::unordered_map<std::string, double> &local_variables,
                    std::unordered_map<std::string, double> &global_variables,
                    const std::string &production) {
  std::string evaluated_production{};
  int index = 0;

  while (index < production.size()) {
    char symbol = production[index++];
    evaluated_production += symbol;

		if (symbol == '[' || symbol == ']') { continue; }
		if (symbol == ' ') { continue; }

		auto symbol_category = get_symbol_category(symbol);
		if (!symbol_category) { 
			LOG_ERROR(app::context.logger, "invalid symbol");
			return std::unexpected(Error::InvalidArgument);
		}
		double symbol_default = symbol_defaults[symbol_category.value()];

    if (index < production.size()) {
      if (production[index] == '{') {
        auto arg_block = get_arg_block(production, index);
        if (!arg_block) {
          return std::unexpected(arg_block.error());
        }

        auto result_2 = evaluate_arg_block(local_variables, global_variables,
                                           arg_block.value());
        if (!result_2) {
          return std::unexpected(result_2.error());
        }

        std::vector<double> args = result_2.value();
        evaluated_production += "{";
        for (int i = 0; i < args.size(); i++) {
          if (i + 1 < args.size()) {
            evaluated_production += fmt::format("{};", args[i]);
          } else {
 evaluated_production += fmt::format("{}", args[i]);
          }
        }
        evaluated_production += "}";

        index += arg_block.value().size();
      } else {
        evaluated_production += fmt::format("{}{}{}", '{', symbol_default, '}');
      }
    } else {
      evaluated_production += fmt::format("{}{}{}", '{', symbol_default, '}');
      break;
    }
  }
	fmt::print("evalprod: {}\n", evaluated_production);
  return evaluated_production;
}

std::expected<std::vector<std::string>, Error>
split_arg_block(const std::string &arg_block) {
	if (arg_block.empty() || arg_block.front() != '{' || arg_block.back() != '}') {
		// throw std::runtime_error("test");
		LOG_ERROR(app::context.logger, "arg_block invalid: {}", arg_block);
		return std::unexpected(Error::InvalidArgument); 
	}
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


std::expected<std::string, Error>
get_arg_block(const std::string lstring, const int index) {
	if (lstring.empty() || index >= lstring.size() || lstring[index] != '{')  {

		LOG_ERROR(app::context.logger, "invalid arguments");
		return std::unexpected(Error::InvalidArgument);
	}

	std::string arg_block{};
	int new_index = index;
	for (; lstring[new_index] != '}'; new_index++) {
		if (new_index >= lstring.size()) { 
			LOG_ERROR(app::context.logger, "'}' not found");
			return std::unexpected(Error::NotFound);
		}
		arg_block += lstring[new_index];
	}
	arg_block += lstring[new_index];

	util::trim(arg_block);
	return arg_block;
}


std::expected<std::vector<double>, Error>
parse_arg_block(const std::string arg_block) {
	auto result = split_arg_block(arg_block);
	if (!result) { return std::unexpected(result.error()); }
	std::vector<std::string> string_args = result.value();

	std::vector<double> args{};
	for (int i = 0; i < string_args.size(); i++) {
		char* end = nullptr;
		double value = std::strtod(string_args[i].c_str(), &end);
		if (end == string_args[i].c_str() || *end != '\0') {
			LOG_ERROR(app::context.logger, "strtod conversion error");
			return std::unexpected(Error::Syntax);
		}
		args.push_back(value);
	}
	return args;
}

std::expected<std::vector<double>, Error>
evaluate_arg_block(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& arg_block) {
	std::vector<double> args{};
	auto result = split_arg_block(arg_block);
	if (!result) { return std::unexpected(result.error()); }
	std::vector<std::string> string_args = result.value();

	for (int i = 0; i < string_args.size(); i++) {
		auto result = evaluate_expression(local_variables, global_variables, string_args[i]);
		if (!result) {
			LOG_WARNING(app::context.logger, "arg_block evaluation failed");
			return std::unexpected(result.error());
		}
		double value = result.value();
		args.push_back(value);
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

	// std::string arg_block = get_arg_block(lstring, local_index);
	// if (arg_block.empty()) { 
	// 	print_info("Arg block empty");
	// 	return std::nullopt; 
	// }
	//
	// local_index += arg_block.size();
	// args = parse_arg_block(arg_block);
	// if (args.empty()) { 
	// 	print_info("Args could not be parsed");
	// 	return std::nullopt;
	// }
	
	return local_index;
}

std::unordered_map<std::string, double> args_to_map(std::vector<double> args) {
	std::unordered_map<std::string, double> args_map{};
	if (args.size() > 0) args_map["x"] = args[0];
	if (args.size() > 1) args_map["y"] = args[1];
	if (args.size() > 2) args_map["z"] = args[2];
	return args_map;
}

std::expected<std::string, Error>
maybe_apply_rule(Generator* generator, const char symbol, std::string arg_block) {
	
	std::unordered_map<std::string, double> local_variables{};
	if (arg_block.empty()) {
	} else {
		std::expected<std::vector<double>, Error> args = parse_arg_block(arg_block);
		if (!args) { return std::unexpected(args.error()); }

		local_variables = args_to_map(args.value());
	}


	// ---- try to match a rule ----
	for (auto &production : generator->productions) {
		std::string condition = production.condition;
		std::string rule = production.rule;

		if (rule.empty()) { continue; }

		if (production.symbol != symbol) { continue; }
				LOG_INFO(app::context.logger, "symbol match");


		if (!condition.empty()) {
			auto result = evaluate_expression(local_variables, generator->global_variables, condition);
			if (!result) {
				LOG_WARNING(app::context.logger, "condition evaluation failed");
				// TODO: draw rule text in red
				continue;
			}
			if (result.value() == false) {
				continue;
			}
		}

		LOG_INFO(app::context.logger, "condition match");

		// ---- rule matched -> replace symbol with rule ----
		auto result = evaluate_production(generator->symbol_defaults, 
				local_variables, generator->global_variables, rule);
		if (!result) { return std::unexpected(result.error()); }
		return result.value();
	}
	if (arg_block.empty()) {
		return fmt::format("{}", symbol);
	} else {
		return fmt::format("{}{}", symbol, arg_block);
	}
	// return fmt::format("
}


std::expected<void, Error>
reset_generator(Generator* generator) {
	generator->clear();
	std::string axiom = generator->axiom;
	std::unordered_map<std::string, double> local_variables{};

	auto result = evaluate_production(generator->symbol_defaults,
			local_variables, generator->global_variables, axiom);
	if (!result) {
		// TODO mark axiom red
		return std::unexpected(result.error());
	}

	generator->lstring = result.value();
	return {};
}


std::expected<bool, Error>
generate_timed(Generator* generator) {
	auto& iterations = generator->iterations;
	auto& current_iteration = generator->current_iteration;
	for (;current_iteration < iterations; current_iteration++) {
		auto result = _expand_lstring(generator);
		if (!result) { return std::unexpected(result.error()); }
		if (!result.value()) { return false; }
	}
	return true;
}


std::expected<bool, Error>
_expand_lstring(Generator* generator) {
	int index = generator->current_index;
	std::string& lstring = generator->lstring;
	char symbol{};

  while (index < lstring.size()) {

    // ---- check time ----
    util::ms elapsed = util::Clock::now() - app::context.frame_start;
    if ((elapsed / app::context.frame_time) >= 0.6) {
      generator->current_index = index;
      return false;
    }

		symbol = lstring[index];
		if (index + 1 < lstring.size()) {
			if (lstring[index + 1] == '{') {
				index++;
				auto arg_block = get_arg_block(lstring, index);
				if (!arg_block) { return std::unexpected(arg_block.error()); }

				auto result = maybe_apply_rule(generator, symbol, arg_block.value());
				if (!result) { return std::unexpected(result.error()); }
				generator->lstring_buffer += result.value();

				index += arg_block.value().size();
			} else {
				auto result = maybe_apply_rule(generator, symbol, std::string{});
				if (!result) { return std::unexpected(result.error()); }
				generator->lstring_buffer += result.value();
				index++;
			}
		} else {
			auto result = maybe_apply_rule(generator, symbol, std::string{});
			if (!result) { return std::unexpected(result.error()); }
			generator->lstring_buffer += result.value();
			break;
		}
	}

  // ---- expansion completed ----
  generator->current_index = 0;
  generator->lstring = generator->lstring_buffer;
  generator->lstring_buffer.clear();
  return true;
}


std::expected<void, Error>
update_generator(Generator* generator) {

	// if reset is needed, reset, then return, not timed,  never takes that long
	if (generator->reset_needed == true) {
		generator->reset_needed = false;
		auto result = reset_generator(generator);
		if (!result) { return std::unexpected(result.error()); }
		generator->done_generating = false;
		LOG_INFO(app::context.logger, "reset finished");
	}

	if (!generator->done_generating) {
		auto result = generate_timed(generator);
		if (!result) { return std::unexpected(result.error()); }
		if (result.value() == true) {
			generator->reset_plants();
			generator->done_generating = true;
			app::context.clear_bg = true;
			LOG_INFO(app::context.logger, "generation finished");
		}
	}
	return {};
}

void _grow(Plant& plant, const float length) {
	Plant::Data& data = plant.data;
	Vec2 old_position = data.position;
	data.position.x += length * cos(data.angle);
	data.position.y += length * -sin(data.angle);
	plant.branches.push_back({old_position, data.position, data.width});
}
void _jump(Plant& plant, const float length) {
	Plant::Data& data = plant.data;
	data.position.x += length * cos(data.angle);
	data.position.y += length * -sin(data.angle);
}

void _turn(Plant::Data& data, const float angle) { data.angle += angle; }
void _change_width(Plant::Data& data, const float width) { data.width += width; }

std::expected<void, Error>
symbol_action(Plant& plant, const char symbol, std::string& arg_block) {
	Plant::Data& data = plant.data;
	std::vector<Plant::Data>& data_stack = plant.data_stack;

	double x{};
	if (!arg_block.empty()) {
		auto args = parse_arg_block(arg_block);
		if (!args) { return std::unexpected(args.error()); }
		x = args.value().front();
	}


	auto symbol_category = get_symbol_category(symbol);
	if (symbol_category == std::nullopt) { return std::unexpected(Error::Generic); }

	// TODO: visable not used
	if (symbol_category.value() == SymbolCategory::Move) {
		if (std::islower(symbol)) {
			_jump(plant, x);
		} else {
			_grow(plant, x);
		}
	}

	// turn turtle counter-clockwise
	if (symbol == '-') {
    _turn(data, -x);
	}
	// turn turtle clockwise
	if (symbol == '+') {
    _turn(data, x);
	}

	// TODO
	// this should either set or change the width
	if (symbol == '^') {
		_change_width(data, -x);
	}
	if (symbol == '&') {
		_change_width(data, x);
	}

	// push and pop turtle state
	if (symbol == '[') {
    data_stack.push_back(data);
	}
  if (symbol == ']') {
    data = data_stack.back();
    data_stack.pop_back();
	}
	return {};
}


// ---- building ----
std::expected<bool, Error>
build_timed(Plant& plant) {
	int index = plant.current_index;

	std::string_view lstring{};
	if (!plant.generator) {
		lstring = plant.lstring;
	} else {
		lstring = plant.generator->lstring;
	}

	char symbol{};

  while (index < lstring.size()) {

    // ---- check time ----
    util::ms elapsed = util::Clock::now() - app::context.frame_start;
    if ((elapsed / app::context.frame_time) >= 0.6) {
      plant.current_index = index;
      return false;
    }

		symbol = lstring[index++];
		if (index < lstring.size() && lstring[index] == '{') {
			auto arg_block = get_arg_block(lstring.data(), index);
			if (!arg_block) { return std::unexpected(arg_block.error()); }

			auto result = symbol_action(plant, symbol, arg_block.value());
			if (!result) { return std::unexpected(result.error()); }

			index += arg_block.value().size();
		} else {
			if (symbol == ' ') {
				continue;
			}
			if (symbol == '[' || symbol == ']') {
				std::string empty_string{};
				auto result = symbol_action(plant, symbol, empty_string);
				if (!result) { return std::unexpected(result.error()); }
				continue;
			}
			LOG_ERROR(app::context.logger, "lstring syntax invalid");
			return std::unexpected(Error::Syntax);
		}
	}

  // ---- expansion completed ----
  plant.current_index = 0;
  return true;
}

std::expected<bool, Error>
draw_plant_timed(Plant& plant, draw::FrameBuf& fb) {

	// LOG_INFO(app::context.logger, "branches count {}", plant.branches.size());
	// LOG_INFO(app::context.logger, "branch: {}", plant.current_branch);
	// draw::wide_line(fb, Line2{{200.0, 000.0}, {400.0, 400.0}}, 0xFFFFFFFF, 5.0);
	for (int i = plant.current_branch; i < plant.branches.size(); i++) {

    // ---- check time ----
    util::ms elapsed = util::Clock::now() - app::context.frame_start;
    if ((elapsed / app::context.frame_time) >= 0.9) {
      plant.current_branch = i;
			LOG_INFO(app::context.logger, "time");
      return false;
    }

		auto &branch = plant.branches[i];
		// LOG_INFO(app::context.logger, "branches {},{},{},{}", branch.start.x, branch.start.y,
				// branch.end.x, branch.end.y);
		draw::wide_line(fb, Line2{branch.start, branch.end}, branch.color, branch.width);
	}
	plant.current_branch = 0;
	return true;
}

std::expected<void, Error>
update_plant(Plant* plant, draw::FrameBuf& fb) {

	// if the lstring the plant points to was changed and is finished
	if (plant->reset_needed == true) {
		plant->reset_needed = false;
		plant->reset();
		LOG_INFO(app::context.logger, "plant reset finished");
		return {};
	}

	if (!plant->done_building) {
		auto result = build_timed(*plant);
		if (!result) { return std::unexpected(result.error()); }
		if (result.value() == true) {
			plant->done_building = true;
			LOG_INFO(app::context.logger, "plant done building");
		}
	}

	// those two logic checks together are wack
	if (!plant->branches.empty() && !plant->done_drawing) {
		auto result_2 = draw_plant_timed(*plant, fb);
		if (!result_2) { return std::unexpected(result_2.error()); }
		if (result_2.value() == true) {
			if (plant->done_building) {
				plant->done_drawing = true;
				LOG_INFO(app::context.logger, "plant done drawing");
			}
		}
	}
	return {};
}

} // namespace lsystem_new
