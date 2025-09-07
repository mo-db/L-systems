// app.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"

namespace app {
struct Context {
  util::ms frame_time = util::ms(1000.0 / 60.0);
	util::TimePoint frame_start;
	bool keep_running = true;
	std::string exec_path = "";
	std::string save_path = "";
	std::string render_path = "";
};

struct Video {
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* window_texture = nullptr;
	// TODO CHANGE
	uint32_t *window_texture_pixels = nullptr;
	// ...
	const SDL_PixelFormatDetails *pixel_format = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);
	int pitch = 0;
	int width = 0;
	int height = 0;
	double window_scale;
	// viewport
	// can both be stored as a vector
	Vec2 screen_offset{};
	double screen_scale_x = 0.0;
	double screen_scale_y = 0.0;
};

struct Gui {
	ImGuiIO *io = nullptr;
	static constexpr int textfield_size = 512;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// windows
	bool show_window_a = false;
	bool show_window_b = false;
	bool show_window_c = false;
};

struct Input {
	Vec2 mouse{};
	bool mouse_left_down = false;
	bool mouse_right_down = false;
	bool mouse_click = false;
	bool shift_set = false;
	bool ctrl_set = false;
};

inline Context context;
inline Video video;
inline Gui gui;
inline Input input;

void init(int width, int height);
} // namespace app
