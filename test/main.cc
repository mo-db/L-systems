#include <print>

constexpr int symbols_quantity = 1000000;
constexpr double val = 123.123456789;

int main() {
	std::FILE* file_stream{std::fopen("string.txt", "w")};
	for (int i = 0; i < symbols_quantity; i++) {
		std::print(file_stream, "A({};{};{})", val, val, val);
	}
	std::fclose(file_stream);
	return 0;
}
