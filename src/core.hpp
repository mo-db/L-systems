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
#include <SDL3_image/SDL_image.h>
#include <fmt/core.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "header_only_libs/exprtk.hpp"
#include "header_only_libs/backward.hpp"


#define print_info(msg) \
    std::cout << "INFO:" << __FILE__ << ":" << __LINE__ << ":" << __func__ << "(), " << msg << std::endl;

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

// colors are ABGR -> little endian with SDL_PIXELFORMAT_RGBA32
constexpr uint32_t black =					0xFF000000;
constexpr uint32_t white =					0xFFFFFFFF;
constexpr uint32_t dark_grey =			0xFF555555;
constexpr uint32_t light_grey =			0xFFCCCCCC;

constexpr uint32_t red =						0xFF0000FF;
constexpr uint32_t green =					0xFF00FF00;
constexpr uint32_t blue =						0xFF0000FF;

constexpr uint32_t yellow =					0xFF00FFFF;
constexpr uint32_t magenta =				0xFFFF00FF;
constexpr uint32_t cyan =						0xFFFFFF00;

constexpr uint32_t fg =							white;
constexpr uint32_t bg =							black;
constexpr uint32_t conceal =				dark_grey;
constexpr uint32_t select =					red;
constexpr uint32_t special = 				blue;
constexpr uint32_t hl_primary =			green;
constexpr uint32_t hl_secondary =		cyan;
constexpr uint32_t hl_tertiary =		magenta;
} // namespace color

namespace util {
// time helper
using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using ms = std::chrono::duration<double, std::milli>;

enum class STATE {
	TRUE,
	FALSE,
	ERROR,
};
inline void toggle(bool &b) { 
	b = !b;
}
inline bool equal_epsilon(double x, double y) {
	return (x < y + gk::epsilon && x > y - gk::epsilon);
}
inline bool equal_iepsilon(double x, double y) {
	return (x < y + gk::iepsilon && x > y - gk::iepsilon);
}
inline std::string get_substr(const std::string &str, const int index, const char c) {
	int field_start = index;
	int field_end = str.find(c, index);
	std::string substr = "";
	if (field_end != std::string::npos) {
		substr = str.substr(index, field_end - field_start);
	}
	return substr;
}

inline std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && s[start] == ' ') {
        ++start;
    }

    size_t end = s.size();
    while (end > start && s[end - 1] == ' ') {
        --end;
    }

    return s.substr(start, end - start);
}
} // namespace util
