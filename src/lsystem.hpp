// lsystem.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"

struct Branch {
	Line2 line;
	uint32_t color;
	float wd;
};

struct Lsystem {
	std::string alphabet = "A,a,B,b,+,-,";
	std::string axiom = "";
	std::string rule_A = "";
	int iterations = 0;
	float default_distance = 50.0;
};

namespace lsystem {
std::vector<Branch> generate_plant();
std::string generate_lstring();
std::string generate_axiom();
std::string generate_rule_A();
}
