#include <string>
#include <iostream>

int main() {
	std::string core = "base|pattern|{repeat}[scale]";
	int sep = core.find('|');
	std::cout << sep << std::endl;

	std::string base = core.substr(0, sep);
	std::cout << base << std::endl;
}
