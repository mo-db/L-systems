// app.hpp
#pragma once
#include "core.hpp"
#include "modules.hpp"
#include "graphics.hpp"

namespace app {
struct Context {
	bool keep_running = true;
	std::string exec_path = "";
	std::string save_path = "";
};

struct Video {
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* window_texture = nullptr;
	uint32_t *frame_buf = nullptr;
	int pitch = 0;
	int width = 0;
	int height = 0;
	double window_scale;
};

struct Gui {
	ImGuiIO *io = nullptr;
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
inline Context video;
inline Context gui;
inline Context input;

void init(int width, int height);
void process_events();
void lock_frame_buf();
void update_gui(Modules &modules);
void render();
void cleanup();
} // namespace app
