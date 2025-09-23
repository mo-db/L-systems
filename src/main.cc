#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
// #include "lsystem.hpp"
#include "lsystem_new.hpp"
#include <lo/lo.h>


void process_events();
void lock_frame_buf();
bool update_gui();
void render();
void cleanup();

int send(void) {
    lo_address addr = lo_address_new("127.0.0.1", "7400");
    float thickness = 0.123f;
    int rc = lo_send(addr, "/fractal/thickness", "f", thickness);
    if (rc == -1) {
        fprintf(stderr, "send failed\n");
        return 1;
    }
    lo_address_free(addr);
    return 0;
}

// store lines in main? vector<line>
int main(int argc, char *argv[]) {
	app::init(960, 540);

	// this should be per mouse click in the gui
	// [add new system] -> select start-point

	// lm::plant.init(Vec2{(double)app::video.width/2, app::video.height - 50.0}, gk::pi / 2);
	// fmt::print("bytes: {}\n", lm::plant.max_nodes * sizeof(Vec2));

	app::gui.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	app::gui.show_window_a = true;
	app::gui.show_window_b = true;
	app::gui.show_window_c = true;
	app::gui.show_rendering_window = true;
	app::gui.show_lsystem_window = true;

	draw::FrameBuf fb_main{app::video.window_texture_pixels, app::video.width, app::video.height};

	while(app::context.keep_running) {
		app::context.frame_start = util::Clock::now();
		app::update_state_queues();
		process_events();
		if (!update_gui()) {
			return 1;
		}

		// where to put this?
		// -> if strg-down on mouse down save mouse coords as old offset cords
		// -> every frame calculate offset with old mouse coords to new mouse
		// position
		if (!viewport::update_vars()) {
			// puts("wtf?");
		}
		// fmt::print("offset: {},{}\n", viewport::vars.xy_offset.x, viewport::vars.xy_offset.y);
		// panning must be changed so it doesnt redraw, it has to use pixels and scale
		if (viewport::vars.panning_active) {
			for (auto &[key, module] : lsystem_new::modules) {
				module.plant.needs_regen = true;
			}
			puts("pan active");
		}

		// ---- update the global and default variables -> on gui event? ----
		for (auto &[key, module] : lsystem_new::modules) {
			lsystem_new::update_vars(module);
			auto &plant = module.plant;

			// ---- generate plant over one or more frames ----
			if (plant.needs_regen || plant.regenerating) {
				if (plant.needs_regen) {
					print_info("clear plant");
					plant.needs_regen = false;
					plant.needs_redraw = true;
					plant.current_lstring_index = 0;
					plant.clear();
				}
				bool done = generate_plant_timed(module);
				if (done) {
					plant.regenerating = false;
				} else {
					done = generate_plant_timed(module);
					plant.regenerating = true;
				}
				lsystem_new::plants_need_redraw = true;
				// plant.redrawing = true; // plant_check if redraw
			}

			// ---- draw plant over one or more frames ----
			// if (plant.needs_redraw || plant.redrawing) {
			// 	// how often to clear and the opacity of the plant should be a setting
			// 	if (plant.needs_redraw) {
			// 		print_info("clear texture");
			// 		plant.needs_redraw = false;
			// 		plant.current_branch = 0;
			// 		draw::clear(fb_main, color::bg);
			// 	}
			// 	bool done = lsystem_new::draw_plants_timed(plant, color::fg, fb_main);
			// 	if (done) {
			// 		plant.redrawing = false;
			// 	} else {
			// 		plant.redrawing = true;
			// 	}
			// }
		}

		if (lsystem_new::plants_need_redraw || lsystem_new::plants_redrawing) {
			if (lsystem_new::plants_need_redraw) {
				lsystem_new::plants_need_redraw = false;
				lsystem_new::plants_drawn = false;
				draw::clear(fb_main, color::bg);
				for (auto &[key, module] : lsystem_new::modules) {
					module.plant.current_branch = 0;
				}
			}
			bool done = lsystem_new::draw_plants_timed(fb_main);
			if (done) {
				lsystem_new::plants_redrawing = false;
			} else {
				lsystem_new::plants_redrawing = true;
			}
		}


		// ---- push framebuffer and render gui ----
		render();
	}
	cleanup();
	return 0;
}



void process_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);

		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
				event.window.windowID == SDL_GetWindowID(app::video.window)) {
      app::context.keep_running = false;
		}

    switch (event.type) {
    case SDL_EVENT_QUIT:
      app::context.keep_running = false;
      break;

		case SDL_EVENT_KEY_UP:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					app::input.shift.down(false);
					break;
				case SDLK_LCTRL:
					app::input.ctrl.down(false);
					break;
				default: break;
			}
			break;

		case SDL_EVENT_KEY_DOWN:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					app::input.shift.down(true);
					break;
				case SDLK_LCTRL:
					app::input.ctrl.down(true);
					break;
        case SDLK_ESCAPE:
					break;
				default: break;
			}
			break;

     case SDL_EVENT_MOUSE_MOTION:
			if (!app::gui.io->WantCaptureMouse) {
				app::input.mouse.x = round(event.motion.x * app::video.window_scale);
				app::input.mouse.y = round(event.motion.y * app::video.window_scale);
			}
      break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == 1) { app::input.mouse_left.down(true); }
			if (event.button.button == 2) { app::input.mouse_middle.down(true); }
			if (event.button.button == 3) { app::input.mouse_right.down(true); }
      break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == 1) { app::input.mouse_left.down(false); }
			if (event.button.button == 2) { app::input.mouse_middle.down(false); }
			if (event.button.button == 3) { app::input.mouse_right.down(false); }
			break;

		default: break;
		}
	}
}

// Modules are bound to and can be created from ModuleTabs
struct ModuleTab {
	int tab_id{};
	int module_id{};
	ModuleTab(int tab_id_, int module_id_) : tab_id{tab_id_}, module_id{module_id_} {}
};

struct GuiModuleSpec {
	std::vector<ModuleTab> tabs{};
	int next_tab_id{};
	bool wait_for_coordinates{false};
};

bool gui_update_module() {
	return true;
}

bool update_gui() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // window_a
  if (app::gui.show_window_a) {
    ImGui::ShowDemoWindow(&app::gui.show_window_a);
  }

  // ___LSYSTEM_MAIN___
  static bool open = true;
  bool *p_open = &open;
  if (app::gui.show_lsystem_window) {

    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("L-System", &app::gui.show_lsystem_window,
                     ImGuiWindowFlags_MenuBar)) {

    // ___MENU_BAR___
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("Close", "Ctrl+W")) {
            *p_open = false;
          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

			static GuiModuleSpec gui_module_spec{};

			if (gui_module_spec.wait_for_coordinates) {
				if (app::input.mouse_left.just_pressed()) {
					gui_module_spec.wait_for_coordinates = false;
					Vec2 start_point = app::input.mouse;
					int current_module_id = lsystem_new::add_module(lsystem_new::Module{
							lsystem_new::Plant{start_point,  gk::pi / 2}, lsystem_new::LstringSpec{}});
					gui_module_spec.tabs.push_back( ModuleTab{gui_module_spec.next_tab_id++, current_module_id} );
					fmt::print("complexes size: {}\n", lsystem_new::modules.size());
				} else {
				}
			}

			if (ImGui::Button("Create Module")) {
				gui_module_spec.wait_for_coordinates = true;
			}

      static ImGuiTabBarFlags tab_bar_flags =
          ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable |
          ImGuiTabBarFlags_FittingPolicyShrink;
      static bool show_leading_button = true;
      static bool show_trailing_button = true;
      ImGui::Checkbox("Show Leading TabItemButton()", &show_leading_button);
      ImGui::Checkbox("Show Trailing TabItemButton()", &show_trailing_button);

      if (ImGui::BeginTabBar("Complexes", tab_bar_flags)) {

        // Leading TabItemButton(): click the "?" button to open a menu
        if (show_leading_button) {
          if (ImGui::TabItemButton("?", ImGuiTabItemFlags_Leading |
                                            ImGuiTabItemFlags_NoTooltip)) {
            ImGui::OpenPopup("MyHelpMenu");
          }
        }
        if (ImGui::BeginPopup("MyHelpMenu")) {
          ImGui::Selectable("Hello!");
          ImGui::EndPopup();
        }

        // Submit our regular tabs
        for (int i = 0; i < gui_module_spec.tabs.size();) {
          bool open = true;
					ModuleTab &tab = gui_module_spec.tabs[i];
          std::string name = fmt::format("{}", tab.tab_id);
          if (ImGui::BeginTabItem(name.c_str(), &open,
                                  ImGuiTabItemFlags_None)) {


						lsystem_new::Module &module = lsystem_new::modules.at(tab.module_id);
						if (ImGui::Button("Generate L-String")) {
								module.lstring = module.lstring_spec.generate(module.lstring);
						}
						ImGui::SameLine();
						if (ImGui::Button("Expand L-String")) {
								module.lstring = lsystem_new::expand_lstring(module, module.lstring);
							module.plant.needs_regen = true;
						}
						ImGui::SameLine();
						if (ImGui::Button("Clear L-String")) {
							module.lstring_spec.current_iteration = 0;
							module.lstring.clear();
						}
						ImGui::SameLine();
						if (ImGui::Button("Regen Plant")) {
							module.plant.needs_regen = true;
						}
						if (ImGui::Button("Print L-string")) {
							fmt::print("L-string: {}\n", module.lstring);
						}

						// ___AXIOM___
						if (ImGui::TreeNode("Axiom")) {
							ImGui::InputText("text", module.lstring_spec.axiom, app::gui.textfield_size);
							ImGui::TreePop();
						}

						if (ImGui::Button("Add Rule")) {
							module.lstring_spec.add_rule();
						}

						// ___RULES___
						for (int i = 0; i < module.lstring_spec.rules.size(); i++) {
							std::string label = fmt::format("Rule {}", i + 1);
							auto &rule = module.lstring_spec.rules[i];

							// select the symbol the rule works on
							if (ImGui::TreeNode(label.c_str())) {
								// ___SYMBOL_SELECTION_COMNO___
								static ImGuiComboFlags flags = 0;
								std::string symbol_str = fmt::format("{}", rule.symbol);
								const char *combo_preview_value = symbol_str.c_str();
									// &rule.symbol;
								if (ImGui::BeginCombo("symbol", combo_preview_value, flags)) {
									for (int n = 0; n < lsystem_new::symbols.size(); n++) {
										char symbol = lsystem_new::symbols[n];
										const bool is_selected = (symbol == rule.symbol);

										symbol_str = fmt::format("{}", symbol);
										if (ImGui::Selectable(symbol_str.c_str(), is_selected))
											rule.symbol = symbol;

										// Set the initial focus when opening the combo (scrolling +
										// keyboard navigation focus)
										if (is_selected)
											ImGui::SetItemDefaultFocus();
									}
									ImGui::EndCombo();
								}

								ImGui::InputText("condition", rule.textfield_condition,
																 app::gui.textfield_size);
								ImGui::InputText("rule", rule.textfield_rule, 
																 app::gui.textfield_size);
								ImGui::TreePop();
							}
						}

						// ---- default variables ----
						for (int i = 0; i < module.default_vars.size(); i++) {
							lsystem_new::Var &var = module.default_vars[i];
							if (ImGui::Checkbox(fmt::format("{} use slider?", var.label).c_str(),
										&var.use_slider)) {}
							if (var.use_slider) {
								ImGui::InputFloat(fmt::format("{}_min", var.label).c_str(),
										&(var.slider_start));
								ImGui::InputFloat(fmt::format("{}_max", var.label).c_str(),
										&(var.slider_end));
								if (ImGui::SliderFloat(fmt::format("{}_slider", var.label).c_str()
											, &(var.value),
										var.slider_start, var.slider_end)) {
									(var.expr)[0] = '\0';
									module.plant.needs_regen = true;
								}
							} else {
								if (ImGui::InputText(fmt::format("{}_text", var.label).c_str(),
											var.expr, app::gui.textfield_size)) {
									module.plant.needs_regen = true;
								}
							}
						}

						// ---- global variables ----
						for (int i = 0; i < module.global_vars.size(); i++) {
							lsystem_new::Var &var = module.global_vars[i];
							if (ImGui::Checkbox(fmt::format("{} use slider?", var.label).c_str(),
										&var.use_slider)) {}
							if (var.use_slider) {
								// ImGui::SameLine();
								ImGui::InputFloat(fmt::format("{}_min", var.label).c_str(),
										&(var.slider_start));
								ImGui::InputFloat(fmt::format("{}_max", var.label).c_str(),
										&(var.slider_end));
								if (ImGui::SliderFloat(fmt::format("{}_slider", var.label).c_str()
											, &(var.value),
										var.slider_start, var.slider_end)) {
									(var.expr)[0] = '\0';
									module.plant.needs_regen = true;
								}
							} else {
								if (ImGui::InputText(fmt::format("{}_text", var.label).c_str(),
											var.expr, app::gui.textfield_size)) {
									module.plant.needs_regen = true;
								}
							}
						}

						ImGui::EndTabItem();
					}



          if (!open) {
            // active_tabs.erase(active_tabs.begin() + i);
						if (!lsystem_new::remove_module((gui_module_spec.tabs.begin() + i)->module_id)) {
							assert(false && "remove_module fail");
						}
						gui_module_spec.tabs.erase(gui_module_spec.tabs.begin() + i);
          } else {
            i++;
          }
        }
        ImGui::EndTabBar();
      }
    }
    ImGui::End();
  }


  // ___LSYSTEM_MAIN___
  static bool open2 = true;
  bool *p_open2 = &open2;
  if (app::gui.show_window_c) {

    // ___MENU_BAR___
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Example: Simple layout", p_open2,
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

			// instead implement serealization for axiom, rules, vars
			// ---- save files ----
			// static std::vector<std::string> save_file_names;
			// if (auto result = lm::scan_saves()) {
			// 	save_file_names = result.value();
			// }
			//
			//      static ImGuiComboFlags flags1 = 0;
			// static int save_fname_index = 0;
			//      const char *combo_preview_value1;
			// if (save_file_names.size() > 0) {
			//      	combo_preview_value1 = (save_file_names[save_fname_index]).c_str();
			// } else {
			//      	combo_preview_value1 = "NIL";
			// }
			//      if (ImGui::BeginCombo("files", combo_preview_value1, flags1)) {
			//        for (int n = 0; n < (int)save_file_names.size(); n++) {
			//          const bool is_selected = (save_fname_index == n);
			//
			//          if (ImGui::Selectable(save_file_names[n].c_str(), is_selected)) {
			// 			// execute if gui drop down field is selected
			//            save_fname_index = n;
			// 			if (!lm::load_rule_from_file(lm::system.rules[i],
			// 						save_file_names[save_fname_index])) {
			// 				std::puts("fail, rule couldnt be loaded");
			// 			}
			// 		}
			//
			//          // Set the initial focus when opening the combo (scrolling +
			//          // keyboard navigation focus)
			//          if (is_selected)
			//            ImGui::SetItemDefaultFocus();
			//        }
			//        ImGui::EndCombo();
			// }



			// ---- SAVE_RULE ---- -> this should save whole state instead
			// static constexpr int text_size = app::Gui::textfield_size;
			// static char save_file_name[text_size] = ""; // needs be persistant
			//      ImGui::InputText("save_file", save_file_name, text_size);
			// if (ImGui::Button("Save As")) {
			// 	if (std::strlen(save_file_name) > 0) {
			// 		if (!lm::save_rule_as_file(lm::system.rules[i], save_file_name)) {
			// 			puts("Rule saving failed, no file was created!");
			// 		}
			// 	}
			// }


			// this should not be here as a whole
			// if (ImGui::Button("render to images")) {
			// 	int frames = 320;
			// 	int width = 640;
			// 	int height = 480;
			// 	SDL_Surface *frame_surf = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
			// 	for (int i = 0; i < frames; i ++) {
			// 		lm::system.standard_angle -= 0.003;
			//      	SDL_FillSurfaceRect(frame_surf, nullptr, 0xFF000000);
			// 		lm::system.lstring = lm::generate_lstring();
			// 		lm::system.plant = lm::generate_plant(Vec2{(double)width * 0.5,
			// 				(double)height * 0.9}, "placeholder");
			// 		for (auto &branch : lm::system.lm::plant.branches) {
			// 			draw::wide_line(draw::FrameBuf{(uint32_t*)frame_surf->pixels,
			// 					frame_surf->w, frame_surf->h}, 
			// 					Line2{*branch.n1, *branch.n2}, 0xFFFF00FF, lm::system.standard_wd);
			// 		}
			//
			// 		fmt::print("{}/frame{:06}\n", app::context.render_path, i);
			// 		std::string frame_file = fmt::format("{}/frame{:06}.png", app::context.render_path, i);
			// 		assert(IMG_SavePNG(frame_surf, frame_file.c_str()));
			// 		// assert(SDL_SaveBMP(frame_surf, frame_file.c_str()));
			// 	}
			// 	SDL_DestroySurface(frame_surf);
			// }
		}
    ImGui::End();
  }
	return true;
}

void render() {
	SDL_SetRenderDrawColor(app::video.renderer, 0, 0, 0, 255);
  SDL_RenderClear(app::video.renderer);

	SDL_UpdateTexture(app::video.window_texture, nullptr, app::video.window_texture_pixels, app::video.width * 4);
  SDL_RenderTexture(app::video.renderer, app::video.window_texture, NULL, NULL);

  ImGui::Render();
  SDL_SetRenderScale(app::video.renderer, app::gui.io->DisplayFramebufferScale.x,
                     app::gui.io->DisplayFramebufferScale.y);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                        app::video.renderer);

	SDL_SetRenderDrawBlendMode(app::video.renderer, SDL_BLENDMODE_NONE);
  SDL_RenderPresent(app::video.renderer);
}

void cleanup() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(app::video.renderer);
  SDL_DestroyWindow(app::video.window);
  SDL_Quit();
}

