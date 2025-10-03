#include "core.hpp"
#include <source_location>

std::expected<double, Error>
do_stuff(int value) {
	if (value < 1) {
		return std::unexpected(Error{error_msg("foo failed")});
	} else if (value > 10) {
		// throw std::runtime_error(format_crash("bar failed"));
		return std::unexpected(Error{});
	}
	return --value;
}

std::expected<int, Error>
process_value(int value) {
	auto result = do_stuff(value);
	// if (!result) { return std::unexpected{result.error()}; }
	if (!result) { 
		// return std::unexpected(Error{
			// fmt::format("{}:{}", __func__, result.error().message)}); }
		return std::unexpected(result.error());
	}
	int processed_value = *result;
	return processed_value;
}

std::expected<int, Error>
outer_function(int value) {
	auto result = process_value(value);
	if (!result) { 
		// error_print(result.error().message);
		// print_info(result.error().message);
		print_info(result.error().message);
		// fmt::print("debug: {}\n", result.error().message);
		return std::unexpected{result.error()};
	}
	int final_value = *result;
	return final_value;
}

int main() { 
	try {
		for (int i = 0; i < 13; i++) {
			int value = outer_function(i).value_or(-1);
			fmt::print("endresult: {}\n", value);
		}
	} catch (const std::runtime_error& e) {
		fmt::print("{}\n", e.what());
	}
	return 0;
}
