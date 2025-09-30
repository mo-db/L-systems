#include "app.hpp"

namespace app {
void init(int width, int height) {

	// init SDL3
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD));
	app::video.window = NULL;
	app::video.renderer = NULL;
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_HIGH_PIXEL_DENSITY |
                                 SDL_WINDOW_MOUSE_CAPTURE;

  assert(SDL_CreateWindowAndRenderer(
      "main", (width), (height),
      window_flags, &app::video.window, &app::video.renderer));
  SDL_SetRenderVSync(app::video.renderer, 1);
  SDL_SetWindowPosition(app::video.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_GetRenderOutputSize(app::video.renderer, &app::video.width, &app::video.height);
	app::video.window_scale = SDL_GetWindowDisplayScale(app::video.window);

  // texture create with pixels and not window size . retina display scaling
  app::video.window_texture = SDL_CreateTexture(
      app::video.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
      app::video.width, app::video.height);
  assert(app::video.window_texture);
	app::video.window_texture_pixels = new uint32_t[app::video.width * app::video.height];

	// set the file system paths
	app::context.exec_path = SDL_GetBasePath();
	// std::filesystem::path saves = (base / ".." / ".." / "saves").lexically_normal();
	app::context.save_path = fmt::format("{}/saves", app::context.exec_path);
	app::context.render_path = fmt::format("{}/renders", app::context.exec_path);

	// init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	app::gui.io = &ImGui::GetIO();
	app::gui.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	app::gui.io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	// Had to change scaling mode in imgui sdlrenderer3 backend
	// -> SDL_SetTextureScaleMode(bd->FontTexture, SDL_SCALEMODE_NEAREST);
	ImGuiStyle& style = ImGui::GetStyle();
	app::gui.io->FontGlobalScale = 0.5f;
	style.ScaleAllSizes(app::video.window_scale);

	app::gui.io->Fonts->AddFontFromFileTTF("/Users/moritz/Library/Fonts/IosevkaFixedSS14-Oblique.ttf", 30.0f);

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(app::video.window, app::video.renderer);
	ImGui_ImplSDLRenderer3_Init(app::video.renderer);
}

void update_state_queues() {
	// input states
	input.mouse_left.update();
	input.mouse_middle.update();
	input.mouse_right.update();
	input.shift.update();
	input.ctrl.update();
}

} // namespace app
