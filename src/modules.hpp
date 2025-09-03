// modules.hpp
#pragma once
#include "core.hpp"

#include "lsystem.hpp"

struct Modules {
	lm::System lsystem{};
	lm::Turtle turtle{};
	std::vector<lm::Turtle> turtle_stack;
};
