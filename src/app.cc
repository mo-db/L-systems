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

	app.context.exec_path = SDL_GetBasePath();
	app.context.save_path = app.context.exec_path + "../../saves/";

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

  // ___LSYSTEM_MAIN___
  static bool open = true;
  bool *p_open = &open;
  if (app.gui.show_window_c) {

    // ___MENU_BAR___
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

      // ___AXIOM___
      if (ImGui::TreeNode("Axiom")) {
        ImGui::InputText("text", lsystem.axiom.text, lsystem.text_size);
        ImGui::TreePop();
      }

			// ___RULES___
      for (int i = 0; i < lsystem.max_rules; i++) {
        std::string label = fmt::format("Rule {}", i + 1);

        if (ImGui::TreeNode(label.c_str())) {
					// drop down menu for saved rule selection
					// gets filled on startup, and if save
					

					std::vector<std::string> save_file_names;
					if (auto result = lsystem::scan_saves()) {
						save_file_names = result.value();
					}

					// for (auto name : save_file_names) {
					// 	std::cout << name << " " << std::endl;
					// }
					
          static ImGuiComboFlags flags1 = 0;
					static int save_fname_index = 0;
          const char *combo_preview_value1;
					if (save_file_names.size() > 0) {
          	combo_preview_value1 = (save_file_names[save_fname_index]).c_str();
					} else {
          	combo_preview_value1 = "NIL";
					}
          if (ImGui::BeginCombo("files", combo_preview_value1, flags1)) {
            for (int n = 0; n < (int)save_file_names.size(); n++) {
              const bool is_selected = (save_fname_index == n);
              if (ImGui::Selectable(save_file_names[n].c_str(), is_selected)) {
								// if mouse select this is run
                save_fname_index = n;
								fmt::print("selected file: {}\n", save_file_names[n]);
								assert(lsystem::load_rule_from_file(lsystem.rules[i], save_file_names[save_fname_index]));
							}

              // Set the initial focus when opening the combo (scrolling +
              // keyboard navigation focus)
              if (is_selected)
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
					}


					// drop down menu for symbol selection
          static ImGuiComboFlags flags = 0;
          const char *combo_preview_value =
              lsystem.alphabet[lsystem.rules[i].symbol_index];
          if (ImGui::BeginCombo("symbol", combo_preview_value, flags)) {
            for (int n = 0; n < lsystem.alphabet_size; n++) {
              const bool is_selected = (lsystem.rules[i].symbol_index == n);
              if (ImGui::Selectable(lsystem.alphabet[n], is_selected))
                lsystem.rules[i].symbol_index = n;

              // Set the initial focus when opening the combo (scrolling +
              // keyboard navigation focus)
              if (is_selected)
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
          }

					// change color based on conditon value
					int colors_pushed = 0;
					if (lsystem.rules[i].condition_state == Lsystem::FIELD_STATE::ERROR) {
						ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
						colors_pushed++;
					} else if (lsystem.rules[i].condition_state == Lsystem::FIELD_STATE::TRUE) {
						colors_pushed++;
						ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.3f, 1.0f, 1.0f));
					}

					// display condition and text
					ImGui::InputText("condition", lsystem.rules[i].condition,
													 lsystem.text_size);
					if (colors_pushed > 0) {
						ImGui::PopStyleColor(colors_pushed);
					}
          ImGui::InputText("text", lsystem.rules[i].text, lsystem.text_size);

					// a button ?
          if (ImGui::Button("Button")) {
            // fmt::print("cond: {}, letter: {}, text {}\n",
            //            lsystem.rules[i].condition,
            //            lsystem.alphabet[lsystem.rules[i].symbol_index],
            //            lsystem.rules[i].text);
            //
						namespace fs = std::filesystem;
						fs::path p = fs::current_path();
						std::string root_path = p;
						fmt::print("relative_path: {}\n", p.relative_path().c_str());
						fmt::print("root_path: {}\n", p.root_path().c_str());
						fmt::print("path: {}\n", p.c_str());
          }
					static char save_file_name[lsystem.text_size] = "";
          ImGui::InputText("save_file", save_file_name, lsystem.text_size);
					if (ImGui::Button("Save As")) {
						fmt::print("save file: {}\n", save_file_name);
						if (std::strlen(save_file_name) > 0) {
							assert(lsystem::save_rule_as_file(lsystem.rules[i], save_file_name)); // dont assert
						}
					}
					// overwrite, later ask if overwrite
					// 	or toogle for overwrite?
          ImGui::TreePop();
        }
      }


			ImGui::Text("nodes: %d", modules.lsystem.plant.node_counter);

			// variables
			ImGui::SliderInt("iterations", &modules.lsystem.iterations, 0, 6);
			ImGui::SliderFloat("float", &modules.lsystem.standard_length, 0.0, 100.0);

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
        static char axiom[32] = "";
        ImGui::InputText("default", axiom, 32);

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
        ImGui::BeginChild("test view",
                          ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        static char axiom[32] = "";
        ImGui::InputText("default", axiom, 32);
        ImGui::EndChild();
      }
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
