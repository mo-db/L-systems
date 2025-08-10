// core.hpp
#pragma once

// types, limits
#include <cstdint>
#include <cstddef>
#include <limits>
#include <numbers>

// containers
#include <array>
#include <vector>

// math, algorithms
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstdlib>

// io, strings
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

// debugging
#include <cassert>
#include <chrono>
#include <expected>
#include <optional>

// SDL
#include <SDL3/SDL.h>
#include <fmt/core.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "exprtk.hpp"
#include "backward.hpp"

inline void print_trace() {
	using namespace backward;
	StackTrace st; st.load_here(32);
	Printer p; p.print(st);
}

namespace gk {
constexpr double epsilon = 1e-6;
constexpr double iepsilon = 0.5;
constexpr double pi = std::numbers::pi;
} // namespace gk

namespace color {

constexpr uint32_t black =					0x000000FF;
constexpr uint32_t white =					0xFFFFFFFF;
constexpr uint32_t dark_grey =			0x005C5C5C;
constexpr uint32_t light_grey =			0x00C5C5C5;

constexpr uint32_t red =						0x00FF0000;
constexpr uint32_t green =					0x0000FF00;
constexpr uint32_t blue =						0x000000FF;

constexpr uint32_t yellow =					0x00FFFF00;
constexpr uint32_t magenta =				0x00FF00FF;
constexpr uint32_t cyan =						0x0000FFFF;

constexpr uint32_t fg =							white;
constexpr uint32_t bg =							black;
constexpr uint32_t conceal =				dark_grey;
constexpr uint32_t select =					red;
constexpr uint32_t special = blue;
constexpr uint32_t hl_primary =			green;
constexpr uint32_t hl_secondary =		cyan;
constexpr uint32_t hl_tertiary =		magenta;
} // namespace color

namespace util {
inline void toggle(bool &b) { 
	b = !b;
}
inline bool equal_epsilon(double x, double y) {
	return (x < y + gk::epsilon && x > y - gk::epsilon);
}
inline bool equal_iepsilon(double x, double y) {
	return (x < y + gk::iepsilon && x > y - gk::iepsilon);
}
} // namespace util
