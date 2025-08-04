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
	app.gui.io->FontGlobalScale = 0.5f;
	style.ScaleAllSizes(app.video.window_scale);

	app.gui.io->Fonts->AddFontFromFileTTF("/Users/moritz/Library/Fonts/IosevkaFixedSS14-Oblique.ttf", 30.0f);

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
	auto &lsystem = modules.lsystem;

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



			if (ImGui::TreeNode("Axiom")) 
			{
				ImGui::InputText("rule", lsystem.axiom, lsystem.alphabet_size);
				ImGui::TreePop();
			}

			for (int i = 0; i < lsystem.max_rules; i++) {
				std::string label = fmt::format("Rule {}", i + 1);

				if (ImGui::TreeNode(label.c_str())) 
				{

					static ImGuiComboFlags flags = 0;
					const char *combo_preview_value = lsystem.alphabet[lsystem.rules[i].letter_index];
					if (ImGui::BeginCombo("symbol", combo_preview_value, flags))
					{
							for (int n = 0; n < lsystem.alphabet_size; n++)
							{
									const bool is_selected = (lsystem.rules[i].letter_index == n);
									if (ImGui::Selectable(lsystem.alphabet[n], is_selected))
											lsystem.rules[i].letter_index = n;

									// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
									if (is_selected)
											ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
					}
					ImGui::InputText("condition", lsystem.rules[i].condition, lsystem.text_size);
					ImGui::InputText("text", lsystem.rules[i].text, lsystem.text_size);
					if (ImGui::Button("Button")) {
						fmt::print("cond: {}, letter: {}, text {}\n", lsystem.rules[i].condition,
								lsystem.alphabet[lsystem.rules[i].letter_index], lsystem.rules[i].text);

						// exprtk just here for testing remove later, implemented in lsys
						typedef double T; // numeric type (float, double, mpfr etc...)
						std::string expression_string = lsystem.rules[i].condition;
						typedef exprtk::symbol_table<T> symbol_table_t;
						typedef exprtk::expression<T>   expression_t;
						typedef exprtk::parser<T>       parser_t;
						T x = T(123.456);
						T y = T(98.98);
						T z = T(0.0);

						symbol_table_t symbol_table;
						symbol_table.add_variable("x",x);
						symbol_table.add_variable("y",y);
						symbol_table.add_variable("z",z);

						expression_t expression;
						expression.register_symbol_table(symbol_table);

						parser_t parser;

						if (!parser.compile(expression_string,expression))
						{
							 printf("Expression Compilation error...\n");
							 return;
						}

						T result = expression.value();
						fmt::print("exprtk result: {}\n", result);

					}
					ImGui::TreePop();
				}
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
        static char axiom[32] = ""; ImGui::InputText("default", axiom, 32);

				if (ImGui::Selectable("rule_A", selected == Selected::RULE_A)) {
					selected = Selected::RULE_A;
				}
				if (ImGui::Button("unselect")) {
					selected = Selected::NONE;
				}

        ImGui::EndChild();
      }
      ImGui::SameLine();
			{
				ImGui::BeginChild("test view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        static char axiom[32] = ""; ImGui::InputText("default", axiom, 32);
				ImGui::EndChild();
			}

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
