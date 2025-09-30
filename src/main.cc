#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "lsystem.hpp"

bool mark_for_regen = false;
bool lstring_live_regen = true;

void process_events();
void lock_frame_buf();
State update_gui(lsystem::LsystemManager& lsystem_manager);
// TODO implement renderer to images and to screen using Dependency Injection
void render();
void cleanup();

// store lines in main? vector<line>
int main(int argc, char *argv[]) {
	app::init(960, 540);
	draw::FrameBuf fb_main{app::video.window_texture_pixels,
												 app::video.width, app::video.height};
  SDL_Surface *frame_surf = 
		SDL_CreateSurface(app::video.width, app::video.height, SDL_PIXELFORMAT_RGBA32);
	draw::FrameBuf framebuffer_image = draw::FrameBuf{(uint32_t*)frame_surf->pixels, frame_surf->w, frame_surf->h};

	lsystem::LsystemManager lsystem_manager{};
	app::gui.show_lsystem_window = true;
	app::gui.show_demo_window = true;

	while(app::context.keep_running) {
		app::context.frame_start = util::Clock::now();
		app::update_state_queues();
		process_events();

		{
			State s = update_gui(lsystem_manager);
			if (s == State::Error) { return 1;}
		}

		{ 
			State s = viewport::update_panning();
			if (s == State::Error) { return 1; }
		}

		// panning must be changed so it doesnt redraw, it has to use pixels and scale
		if (viewport::spec.panning_active) {
			for (auto &[key, module] : lsystem_manager.modules) {
				module->plant.needs_regen = true;
			}
			// puts("pan active");
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

State update_gui(lsystem::LsystemManager& lsystem_manager) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // demo_window
  if (app::gui.show_demo_window) {
    ImGui::ShowDemoWindow(&app::gui.show_demo_window);
  }

  // ___LSYSTEM_MAIN___
  // static bool open = true;
  // bool *p_open = &open;
  // if (app::gui.show_lsystem_window) {
  //
  //   ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
  //   if (ImGui::Begin("L-System", &app::gui.show_lsystem_window,
  //                    ImGuiWindowFlags_MenuBar)) {
  //
  //   // ___MENU_BAR___
  //     if (ImGui::BeginMenuBar()) {
  //       if (ImGui::BeginMenu("File")) {
  //         if (ImGui::MenuItem("Close", "Ctrl+W")) {
  //           *p_open = false;
  //         }
  //         ImGui::EndMenu();
  //       }
  //       ImGui::EndMenuBar();
  //     }
  //
  // 	static GuiModuleSpec gui_module_spec{};
  //
  // 	if (gui_module_spec.wait_for_coordinates) {
  // 		if (app::input.mouse_left.just_pressed()) {
  // 			gui_module_spec.wait_for_coordinates = false;
  // 			int current_module_id = lsystem_manager.add_module(app::input.mouse, gk::pi / 2);
  // 			gui_module_spec.tabs.push_back( ModuleTab{gui_module_spec.next_tab_id++, current_module_id} );
  // 			fmt::print("current module count: {}\n", lsystem_manager.modules.size());
  // 		} else {
  // 		}
  // 	}
  //
  // 	if (ImGui::Button("test")) {
  // 		auto start = util::Clock::now();
  // 		State state = test_evaluate_expression();
  // 		if (state == State::False) {
  // 			fmt::print("Test False\n");
  // 		} else {
  // 			fmt::print("Test True\n");
  // 		}
  // 		util::ms elapsed = util::Clock::now() - start;
  // 		fmt::print("time: {}\n", elapsed.count());
  // 	}
  //
  // 	if (ImGui::Button("Create Module")) {
  // 		gui_module_spec.wait_for_coordinates = true;
  // 	}
  //
  //     static ImGuiTabBarFlags tab_bar_flags =
  //         ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable |
  //         ImGuiTabBarFlags_FittingPolicyShrink;
  //
  //     if (ImGui::BeginTabBar("Modules", tab_bar_flags)) {
  //
  //       // Leading TabItemButton(): click the "?" button to open a menu
  // 		if (ImGui::TabItemButton("?", ImGuiTabItemFlags_Leading |
  // 																			ImGuiTabItemFlags_NoTooltip)) {
  // 			gui_module_spec.wait_for_coordinates = true;
  // 			ImGui::OpenPopup("MyHelpMenu");
  // 		}
  //       if (ImGui::BeginPopup("MyHelpMenu")) {
  //         ImGui::Selectable("Select position!");
  //         ImGui::EndPopup();
  //       }
  //
  //       // Submit our regular tabs
  //       for (int i = 0; i < gui_module_spec.tabs.size();) {
  //         bool open = true;
  // 			ModuleTab &tab = gui_module_spec.tabs[i];
  //         std::string name = fmt::format("{}", tab.tab_id);
  //
  //         if (ImGui::BeginTabItem(name.c_str(), &open, ImGuiTabItemFlags_None)) {
  // 				lsystem::Module* module = lsystem_manager.get_module(tab.module_id);
  // 				assert(module);
  // 				if (ImGui::Button("Expand L-String")) {
  // 					{
  // 						State state = lsystem::expand_lstring(module, true);
  // 						if (state == State::Error) { return state; }
  // 					}
  // 				}
  // 				ImGui::SameLine();
  // 				if (ImGui::Button("Regenerate L-String")) {
  // 					{
  // 						State state = lsystem::regenerate_lstring(module);
  // 						if (state == State::Error) { return state; }
  // 					}
  // 				}
  // 				ImGui::SameLine();
  // 				if (ImGui::Button("Clear L-String")) {
  // 					lsystem::clear_lstring(module);
  // 				}
  // 				ImGui::SameLine();
  // 				if (ImGui::Button("Regen Plant")) {
  // 					module->plant.needs_regen = true;
  // 				}
  // 				ImGui::SameLine();
  // 				ImGui::Text("Current iteration: %d", module->geneartion_manager.current_iteration);
  // 				if (ImGui::Button("Print L-string")) {
  // 					fmt::print("L-string: {}\n", module->lstring);
  // 				}
  //
  //
  // 				// 1. if current iteration is 0
  // 				// 2. every frame check if lstring needs regen
  //
  // 				// ___AXIOM___
  // 				// TODO
  // 				if (ImGui::TreeNode("Axiom")) {
  // 					if (ImGui::InputText("text", module->geneartion_manager.axiom, app::gui.textfield_size)) {
  // 						if (lstring_live_regen) {
  // 							module->geneartion_manager.needs_regen = true;
  // 						}
  // 						// 	State state = lsystem::expand(module, module->geneartion_manager.axiom);
  // 						// 	if (state == State::Error) { return state; }
  // 						// 	module->geneartion_manager.iteration_count = 1;
  // 						// }
  // 					}
  // 					ImGui::TreePop();
  // 				}
  //
  // 				if (ImGui::Button("Add Rule")) {
  // 					module->geneartion_manager.add_rule();
  // 				}
  //
  // 				// ___RULES___
  // 				for (int i = 0; i < module->geneartion_manager.rules.size(); i++) {
  // 					std::string label = fmt::format("Rule {}", i + 1);
  // 					auto &rule = module->geneartion_manager.rules[i];
  //
  // 					// select the symbol the rule works on
  // 					if (ImGui::TreeNode(label.c_str())) {
  // 						// ___SYMBOL_SELECTION_COMNO___
  // 						static ImGuiComboFlags flags = 0;
  // 						std::string symbol_str = fmt::format("{}", rule.symbol);
  // 						const char *combo_preview_value = symbol_str.c_str();
  // 							// &rule.symbol;
  // 						if (ImGui::BeginCombo("symbol", combo_preview_value, flags)) {
  // 							for (int n = 0; n < lsystem::symbols.size(); n++) {
  // 								char symbol = lsystem::symbols[n];
  // 								const bool is_selected = (symbol == rule.symbol);
  //
  // 								symbol_str = fmt::format("{}", symbol);
  // 								if (ImGui::Selectable(symbol_str.c_str(), is_selected))
  // 									rule.symbol = symbol;
  //
  // 								// Set the initial focus when opening the combo (scrolling +
  // 								// keyboard navigation focus)
  // 								if (is_selected)
  // 									ImGui::SetItemDefaultFocus();
  // 							}
  // 							ImGui::EndCombo();
  // 						}
  //
  // 						if (ImGui::InputText("condition", rule.textfield_condition,
  // 														 app::gui.textfield_size)) {
  // 							if (lstring_live_regen) {
  // 								module->geneartion_manager.needs_regen = true;
  // 							}
  // 							// if (module->geneartion_manager.iteration_count <= 1) {
  // 							// 	State state = lsystem::expand(module, module->geneartion_manager.axiom);
  // 							// 	if (state == State::Error) { return state; }
  // 							// 	module->geneartion_manager.iteration_count = 1;
  // 							// }
  // 						}
  // 						if (ImGui::InputText("rule", rule.textfield_rule, 
  // 														 app::gui.textfield_size)) {
  //
  // 							if (lstring_live_regen) {
  // 								module->geneartion_manager.needs_regen = true;
  // 							}
  // 							// if (module->geneartion_manager.iteration_count <= 1) {
  // 							// 	State state = lsystem::expand(module, module->geneartion_manager.axiom);
  // 							// 	if (state == State::Error) { return state; }
  // 							// 	module->geneartion_manager.iteration_count = 1;
  // 							// }
  // 						}
  // 						ImGui::TreePop();
  // 					}
  // 				}
  //
  // 				ImGui::SliderInt("Iterations", &module->geneartion_manager.iterations, 1, 16);
  //
  // 				// ---- default variables ----
  // 				for (int i = 0; i < module->default_vars.size(); i++) {
  // 					lsystem::Var &var = module->default_vars[i];
  // 					if (ImGui::Checkbox(fmt::format("{} use slider?", var.label).c_str(),
  // 								&var.use_slider)) {}
  // 					if (var.use_slider) {
  // 						ImGui::InputFloat(fmt::format("{}_min", var.label).c_str(),
  // 								&(var.slider_start));
  // 						ImGui::InputFloat(fmt::format("{}_max", var.label).c_str(),
  // 								&(var.slider_end));
  // 						if (ImGui::SliderFloat(fmt::format("{}_slider", var.label).c_str()
  // 									, &(var.value),
  // 								var.slider_start, var.slider_end)) {
  // 							(var.expr)[0] = '\0';
  // 							module->plant.needs_regen = true;
  // 						}
  // 					} else {
  // 						if (ImGui::InputText(fmt::format("{}_text", var.label).c_str(),
  // 									var.expr, app::gui.textfield_size)) {
  // 							module->plant.needs_regen = true;
  // 						}
  // 					}
  // 				}
  //
  // 				// ---- global variables ----
  // 				for (int i = 0; i < module->global_vars.size(); i++) {
  // 					lsystem::Var &var = module->global_vars[i];
  // 					if (ImGui::Checkbox(fmt::format("{} use slider?", var.label).c_str(),
  // 								&var.use_slider)) {}
  // 					if (var.use_slider) {
  // 						// ImGui::SameLine();
  // 						ImGui::InputFloat(fmt::format("{}_min", var.label).c_str(),
  // 								&(var.slider_start));
  // 						ImGui::InputFloat(fmt::format("{}_max", var.label).c_str(),
  // 								&(var.slider_end));
  // 						if (ImGui::SliderFloat(fmt::format("{}_slider", var.label).c_str()
  // 									, &(var.value),
  // 								var.slider_start, var.slider_end)) {
  // 							(var.expr)[0] = '\0';
  // 							module->plant.needs_regen = true;
  // 						}
  // 					} else {
  // 						if (ImGui::InputText(fmt::format("{}_text", var.label).c_str(),
  // 									var.expr, app::gui.textfield_size)) {
  // 							module->plant.needs_regen = true;
  // 						}
  // 					}
  // 				}
  //
  // 				ImGui::EndTabItem();
  // 			}
  //
  //         if (!open) {
  // 				{
  // 				 State state = lsystem_manager.remove_module((gui_module_spec.tabs.begin() + i)->module_id);
  // 				 if (state == State::Error) { return state; }
  // 				}
  // 				gui_module_spec.tabs.erase(gui_module_spec.tabs.begin() + i);
  //         } else {
  //           i++;
  //         }
  //       }
  //       ImGui::EndTabBar();
  //     }
  //   }
  //   ImGui::End();
  // }


  // // ___LSYSTEM_MAIN___
  // static bool open2 = true;
  // bool *p_open2 = &open2;
  // if (app::gui.show_window_c) {
  //
  //   // ___MENU_BAR___
  //   ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
  //   if (ImGui::Begin("Example: Simple layout", p_open2,
  //                    ImGuiWindowFlags_MenuBar)) {
  //     if (ImGui::BeginMenuBar()) {
  //       if (ImGui::BeginMenu("File")) {
  //         if (ImGui::MenuItem("Close", "Ctrl+W")) {
  //           *p_open = false;
  //         }
  //         ImGui::EndMenu();
  //       }
  //       ImGui::EndMenuBar();
  //     }
  //
  // 	// instead implement serealization for axiom, rules, vars
  // 	// ---- save files ----
  // 	// static std::vector<std::string> save_file_names;
  // 	// if (auto result = lm::scan_saves()) {
  // 	// 	save_file_names = result.value();
  // 	// }
  // 	//
  // 	//      static ImGuiComboFlags flags1 = 0;
  // 	// static int save_fname_index = 0;
  // 	//      const char *combo_preview_value1;
  // 	// if (save_file_names.size() > 0) {
  // 	//      	combo_preview_value1 = (save_file_names[save_fname_index]).c_str();
  // 	// } else {
  // 	//      	combo_preview_value1 = "NIL";
  // 	// }
  // 	//      if (ImGui::BeginCombo("files", combo_preview_value1, flags1)) {
  // 	//        for (int n = 0; n < (int)save_file_names.size(); n++) {
  // 	//          const bool is_selected = (save_fname_index == n);
  // 	//
  // 	//          if (ImGui::Selectable(save_file_names[n].c_str(), is_selected)) {
  // 	// 			// execute if gui drop down field is selected
  // 	//            save_fname_index = n;
  // 	// 			if (!lm::load_rule_from_file(lm::system.rules[i],
  // 	// 						save_file_names[save_fname_index])) {
  // 	// 				std::puts("fail, rule couldnt be loaded");
  // 	// 			}
  // 	// 		}
  // 	//
  // 	//          // Set the initial focus when opening the combo (scrolling +
  // 	//          // keyboard navigation focus)
  // 	//          if (is_selected)
  // 	//            ImGui::SetItemDefaultFocus();
  // 	//        }
  // 	//        ImGui::EndCombo();
  // 	// }
  //
  //
  //
  // 	// ---- SAVE_RULE ---- -> this should save whole state instead
  // 	// static constexpr int text_size = app::Gui::textfield_size;
  // 	// static char save_file_name[text_size] = ""; // needs be persistant
  // 	//      ImGui::InputText("save_file", save_file_name, text_size);
  // 	// if (ImGui::Button("Save As")) {
  // 	// 	if (std::strlen(save_file_name) > 0) {
  // 	// 		if (!lm::save_rule_as_file(lm::system.rules[i], save_file_name)) {
  // 	// 			puts("Rule saving failed, no file was created!");
  // 	// 		}
  // 	// 	}
  // 	// }
  //
  //
  // 	// this should not be here as a whole
  // 	// if (ImGui::Button("render to images")) {
  // 	// 	int frames = 320;
  // 	// 	int width = 640;
  // 	// 	int height = 480;
  // 	// 	SDL_Surface *frame_surf = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
  // 	// 	for (int i = 0; i < frames; i ++) {
  // 	// 		lm::system.standard_angle -= 0.003;
  // 	//      	SDL_FillSurfaceRect(frame_surf, nullptr, 0xFF000000);
  // 	// 		lm::system.lstring = lm::generate_lstring();
  // 	// 		lm::system.plant = lm::generate_plant(Vec2{(double)width * 0.5,
  // 	// 				(double)height * 0.9}, "placeholder");
  // 	// 		for (auto &branch : lm::system.lm::plant.branches) {
  // 	// 			draw::wide_line(draw::FrameBuf{(uint32_t*)frame_surf->pixels,
  // 	// 					frame_surf->w, frame_surf->h}, 
  // 	// 					Line2{*branch.n1, *branch.n2}, 0xFFFF00FF, lm::system.standard_wd);
  // 	// 		}
  // 	//
  // 	// 		fmt::print("{}/frame{:06}\n", app::context.render_path, i);
  // 	// 		std::string frame_file = fmt::format("{}/frame{:06}.png", app::context.render_path, i);
  // 	// 		assert(IMG_SavePNG(frame_surf, frame_file.c_str()));
  // 	// 		// assert(SDL_SaveBMP(frame_surf, frame_file.c_str()));
  // 	// 	}
  // 	// 	SDL_DestroySurface(frame_surf);
  // 	// }
  // }
  //   ImGui::End();
  // }
	return State::True;
}


// puth in extra file?

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

