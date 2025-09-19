// lsystem_new.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

namespace lsystem {
struct GlobVars{};
struct DefaultVars{};

struct Args{};
struct Symbols {
	static constexpr int quant = 6;
	const char* A{"A"};
	const char* a{"a"};
	const char* minus{"-"};
	const char* plus{"+"};
	const char* inc_width{"^"};
	const char* dec_width{"&"};
	const char* var(const int i) {
		switch (i) {
			case 0: return A;
			case 1: return a;
			case 2: return minus;
			case 3: return plus;
			case 4: return inc_width;
			case 5: return dec_width;
			default: return nullptr;
		}
	}
};
inline Symbols symbols;

struct Rule {
	int symbol_index = 0;
	char condition[app::gui.textfield_size] = { '\0' };
	util::STATE condition_state = util::STATE::FALSE;
	char text[app::gui.textfield_size] = "";
};
struct GeneratorSpec {};

}
