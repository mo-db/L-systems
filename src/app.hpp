// app.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

enum struct AppMode {
  NORMAL,
};


struct App {

	struct Context {
		AppMode mode = AppMode::NORMAL;
		bool keep_running = true;
	} context;

	struct Video {
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;
		SDL_Texture* window_texture = nullptr;
		int width = 0;
		int height = 0;
		double window_scale;
	} video;

	struct Gui {
    ImGuiIO *io = nullptr;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		// windows
		bool show_window_a = false;
		bool show_window_b = false;
		bool show_window_c = false;
	} gui;

	struct Input {
		Vec2 mouse{};
		bool mouse_left_down = false;
		bool mouse_right_down = false;
		bool mouse_click = false;
		bool shift_set = false;
		bool ctrl_set = false;
	} input;
};

namespace app {
void init(App &app, int width, int height);
void process_events(App &app);
void update_gui(App &app);
int resize(App &app, int width, int height);
} // namespace app
