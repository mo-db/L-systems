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
void update_gui(App &app, Modules &modules) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // window_a
  if (app.gui.show_window_a) {
    ImGui::ShowDemoWindow(&app.gui.show_window_a);
  }

  // window_b
  if (app.gui.show_window_b) {
    static int counter = 0;

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::Checkbox("Demo Window", &app.gui.show_window_a);
    ImGui::Checkbox("Another Window", &app.gui.show_window_b);

    ImGui::SliderInt("iterations", &modules.lsystem.iterations, 0, 6);
    ImGui::SliderFloat("float", &modules.lsystem.standard_length, 0.0, 100.0);

    if (ImGui::Button("Button"))
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / app.gui.io->Framerate, app.gui.io->Framerate);
    ImGui::End();
  }

  // // window_c
  // if (app.gui.show_window_c) {
  //   ImGui::Begin("Another Window", &app.gui.show_window_c);
  //   ImGui::Text("Hello from another window!");
  //   if (ImGui::Button("Close Me"))
  //     app.gui.show_window_c = false;
  //   ImGui::End();
  // }

  static bool open = true;
  bool *p_open = &open;
  // *p_open = true;

  // Demonstrate create a window with multiple child windows.
  if (app.gui.show_window_c) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Example: Simple layout", p_open,
                     ImGuiWindowFlags_MenuBar)) {
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("Close", "Ctrl+W")) {
            *p_open = false;
          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

			enum class Selected {
				NONE,
				AXIOM,
				RULE_A,
			};

      // Left
      static Selected selected = Selected::NONE;
      {
        ImGui::BeginChild("left pane", ImVec2(150, 0),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				if (ImGui::Selectable("axiom", selected == Selected::AXIOM)) {
					selected = Selected::AXIOM;
				}

				if (ImGui::Selectable("rule_A", selected == Selected::RULE_A)) {
					selected = Selected::RULE_A;
				}
				if (ImGui::Button("unselect")) {
					selected = Selected::NONE;
				}

        ImGui::EndChild();
      }
      ImGui::SameLine();

      // Right
      // {
      // 		ImGui::BeginGroup();
      // 		ImGui::BeginChild("item view", ImVec2(0,
      // -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below
      // us 		ImGui::Text("MyObject: %d", selected); 		ImGui::Separator(); 		if
      // (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
      // 		{
      // 				if (ImGui::BeginTabItem("Description"))
      // 				{
      // 						ImGui::TextWrapped("Lorem
      // ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
      // tempor incididunt ut labore et dolore magna aliqua. ");
      // 						ImGui::EndTabItem();
      // 				}
      // 				if (ImGui::BeginTabItem("Details"))
      // 				{
      // 						ImGui::Text("ID:
      // 0123456789"); 						ImGui::EndTabItem();
      // 				}
      // 				ImGui::EndTabBar();
      // 		}
      // 		ImGui::EndChild();
      // 		if (ImGui::Button("Revert")) {}
      // 		ImGui::SameLine();
      // 		if (ImGui::Button("Save")) {}
      // 		ImGui::EndGroup();
      // }
    }
    ImGui::End();
  }
}

void lock_frame_buf(App &app) {
  assert(SDL_LockTexture(app.video.window_texture, NULL,
                         reinterpret_cast<void **>(&app.video.frame_buf),
                         &app.video.pitch));
  std::fill_n(app.video.frame_buf, app.video.width * app.video.height,
              color::bg);
}

void render(App &app) {
  SDL_UnlockTexture(app.video.window_texture);
  SDL_RenderTexture(app.video.renderer, app.video.window_texture, NULL, NULL);
  app.video.frame_buf = nullptr;
  app.video.pitch = 0;

  ImGui::Render();
  SDL_SetRenderScale(app.video.renderer, app.gui.io->DisplayFramebufferScale.x,
                     app.gui.io->DisplayFramebufferScale.y);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                        app.video.renderer);
  SDL_RenderPresent(app.video.renderer);
}

void cleanup(App &app) {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(app.video.renderer);
  SDL_DestroyWindow(app.video.window);
  SDL_Quit();
}
} // namespace app
