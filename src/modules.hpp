// modules.hpp
#pragma once
#include "core.hpp"

#include "lsystem.hpp"

struct Modules {
	Lsystem lsystem;
	Turtle turtle;
	std::vector<Turtle> turtle_stack;
};
