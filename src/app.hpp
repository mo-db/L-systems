// app.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"

namespace app {

// to keep track if a state changed this frame
class StateQueue {
  bool current_state{false};
  bool prev_state{false};

public:
  inline void update() { prev_state = current_state; }
  inline bool state() { return current_state; }
  inline void state(bool state) { current_state = state; }
  inline bool became_true() {
    return (!prev_state && current_state) ? true : false;
  }
  inline bool became_false() {
    return (prev_state && !current_state) ? true : false;
  }
};

// useful to keep track if a button was pressed this frame
class InputStateQueue {
  bool current_state{false};
  bool prev_state{false};

public:
  inline void update() { prev_state = current_state; }
  inline bool down() { return current_state; }
  inline void down(bool state) { current_state = state; }
  inline bool just_pressed() {
    return (!prev_state && current_state) ? true : false;
  }
  inline bool just_released() {
    return (prev_state && !current_state) ? true : false;
  }
};

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
	bool show_demo_window = false;
	bool show_lsystem_window = false;
};

struct Input {
	InputStateQueue mouse_left{};
	InputStateQueue mouse_middle{};
	InputStateQueue mouse_right{};
	InputStateQueue shift{};
	InputStateQueue ctrl{};
	Vec2 mouse{};
};

inline Context context;
inline Video video;
inline Gui gui;
inline Input input;

// extern Context context;
// extern Video video;
// extern Gui gui;
// extern Input input;

void init(int width, int height);
void update_state_queues();
} // namespace app
