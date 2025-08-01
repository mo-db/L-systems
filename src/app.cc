#include "app.hpp"

namespace app {
void init(App &app, int width, int height) {

	// init SDL3
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD));
  app.video.window = NULL;
  app.video.renderer = NULL;
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_HIGH_PIXEL_DENSITY |
                                 SDL_WINDOW_MOUSE_CAPTURE;

  assert(SDL_CreateWindowAndRenderer(
      "main", (width), (height),
      window_flags, &app.video.window, &app.video.renderer));
  SDL_SetRenderVSync(app.video.renderer, 1);
  SDL_SetWindowPosition(app.video.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_GetRenderOutputSize(app.video.renderer, &app.video.width, &app.video.height);
	app.video.window_scale = SDL_GetWindowDisplayScale(app.video.window);

  // texture create with pixels and not window size . retina display scaling
  app.video.window_texture = SDL_CreateTexture(
      app.video.renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
      app.video.width, app.video.height);
  assert(app.video.window_texture);

	// init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	app.gui.io = &ImGui::GetIO();
	app.gui.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	app.gui.io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	// Had to change scaling mode in imgui sdlrenderer3 backend
	// -> SDL_SetTextureScaleMode(bd->FontTexture, SDL_SCALEMODE_NEAREST);
	ImGuiStyle& style = ImGui::GetStyle();
	// app.gui.io->FontGlobalScale = app.video.window_scale;
	style.ScaleAllSizes(app.video.window_scale);

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(app.video.window, app.video.renderer);
	ImGui_ImplSDLRenderer3_Init(app.video.renderer);
}


// could return vector of key presses, maybe pairs of key and state
void process_events(App &app) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);

		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
				event.window.windowID == SDL_GetWindowID(app.video.window)) {
      app.context.keep_running = false;
		}

    switch (event.type) {
    case SDL_EVENT_QUIT:
      app.context.keep_running = false;
      break;

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:

		case SDL_EVENT_KEY_UP:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					if (!event.key.repeat) {
						app.input.shift_set = false;
					}
					break;
				case SDLK_LCTRL:
					if (!event.key.repeat) {
						app.input.ctrl_set = false;
					}
					break;
				default: break;
			}
			break;

		case SDL_EVENT_KEY_DOWN:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					if (!event.key.repeat) {
						app.input.shift_set = true;
					}
					break;
				case SDLK_LCTRL:
					if (!event.key.repeat) {
						app.input.ctrl_set = true;
					}
					break;
        case SDLK_ESCAPE:
					break;
				default: break;
			}
			break;

     case SDL_EVENT_MOUSE_MOTION:
			if (!app.gui.io->WantCaptureMouse) {
				app.input.mouse.x = round(event.motion.x * app.video.window_scale);
				app.input.mouse.y = round(event.motion.y * app.video.window_scale);
			}
      break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      app.input.mouse_left_down = event.button.down;
			app.input.mouse_click = true;
      break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      app.input.mouse_left_down = event.button.down;
      break;

		default: break;
		}
	}
}
void update_gui(App &app) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // window_a
  ImGui::ShowDemoWindow(&app.gui.show_window_a);

  // window_b
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
		ImGui::Checkbox(
        "Demo Window",
        &app.gui
             .show_window_a);
    ImGui::Checkbox("Another Window", &app.gui.show_window_b);

    ImGui::SliderFloat("float", &f, 0.0f,
                       1.0f);

    if (ImGui::Button("Button"))
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / app.gui.io->Framerate, app.gui.io->Framerate);
    ImGui::End();
  }

  // window_c
  if (app.gui.show_window_c) {
    ImGui::Begin("Another Window", &app.gui.show_window_c);
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      app.gui.show_window_c = false;
    ImGui::End();
  }
}

} // namespace app
