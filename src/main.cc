#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "lsystem_new.hpp"
#include "observer.hpp"

bool mark_for_regen = false;
bool lstring_live_regen = true;

void process_events();
void lock_frame_buf();
State update_gui(lsystem_new::LsystemManager& lsystem_manager);
// TODO implement renderer to images and to screen using Dependency Injection
void render();
void cleanup();

State test_parse_symbol(const std::string& lstring) {
	int index{};
	while(index < lstring.size()) {
		char symbol{};
		std::vector<double> args{};
		auto result = lsystem_new::parse_symbol(lstring, index, symbol, args);
		if (result == std::nullopt) { return State::Error; }
		if (result.value() == 0) {
			// symbol is last char
			fmt::print("index-{}: {}\n", index, symbol);
			break;
		}
		fmt::print("index-{}: \n", index);
		for (auto & arg : args) {
			fmt::print("{},", arg);
		}
		fmt::print("\n");
		index = result.value();
	}
	return State::True;
}


// store lines in main? vector<line>
int main(int argc, char *argv[]) {
	try {
		app::init(960, 540);
		app::context.logger->set_log_level(quill::LogLevel::Info);

		draw::FrameBuf fb_main{app::video.window_texture_pixels,
													 app::video.width, app::video.height};
		SDL_Surface *frame_surf = 
			SDL_CreateSurface(app::video.width, app::video.height, SDL_PIXELFORMAT_RGBA32);
		draw::FrameBuf framebuffer_image = draw::FrameBuf{(uint32_t*)frame_surf->pixels, frame_surf->w, frame_surf->h};

		lsystem_new::LsystemManager lsystem_manager{};

		app::gui.show_demo_window = true;
		app::gui.show_generator_window = true;
		app::gui.show_builder_window = true;


		lsystem_manager.add_plant_builder(0);
		


		while(app::context.keep_running) {
			app::context.frame_start = util::Clock::now();
			app::update_state_queues();
			process_events();


			// // CHANGE:
			// if (lsystem_manager.generators.size() == 1 &&
			// 		lsystem_manager.plant_builders.size() == 1) {
			// 	if (lsystem_manager.generators[0]->done_generating) {
			// 		lsystem_manager.plant_builders[0]->plant.reset_needed = true;
			// 		lsystem_manager.plant_builders[0]->plant.lstring =
			// 			lsystem_manager.generators[0]->lstring;
			// 		LOG_INFO(app::context.logger, "plant will reset!");
			// 	}
			// }

			// draw::wide_line(fb_main, Line2{{0.0, 0.0}, {900.0, 400.0}}, 0xFFFFFFFF, 5.0);

			{
				State s = update_gui(lsystem_manager);
				if (s == State::Error) { return 1;}
			}

			{ 
				State s = viewport::update_panning();
				if (s == State::Error) { return 1; }
			}

			for (auto &[id, generator] : lsystem_manager.generators) {
				auto result = lsystem_new::update_generator(generator.get());
			}

			for (auto &[id, builder] : lsystem_manager.plant_builders) {
				for (auto &[id, plant] : builder->plants) {
					auto result = lsystem_new::update_plant(plant.get(), fb_main);
				}
				// auto result = lsystem_new::update_builder(builder.get(), fb_main);
			}

			// panning must be changed so it doesnt redraw, it has to use pixels and scale
			if (viewport::spec.panning_active) {
				// for (auto &[key, module] : lsystem_manager.modules) {
				// 	module->plant.needs_regen = true;
				// }
				// puts("pan active");
			}

			// ---- push framebuffer and render gui ----
			render();
			// catch (const std::runtime_error& e) {
			// 	int i = 0;
			// }
		}
	} catch (const std::runtime_error& e) {
		LOG_CRITICAL(app::context.logger, "RuntimeError: {}", e.what());
		cleanup();
		return 1;
	} catch (const std::logic_error& e) {
		LOG_CRITICAL(app::context.logger, "LogicError: {}", e.what());
		cleanup();
		return 1;
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

struct GuiVariable {
	bool use_slider{false};
	float slider_start{0.0};
	float slider_end{10.0};
	char expression[lsystem_new::textfield_size]{};
	GuiVariable() = default;
	GuiVariable(const double slider_end_) : slider_end(slider_end_) {}
};

// this should be generated on init

struct GeneratorWindowTab {
	int id{};
	GeneratorWindowTab(const int id_) :id{id_} {}
	std::unordered_map<lsystem_new::SymbolCategory, GuiVariable> symbol_defaults{};
	std::unordered_map<std::string, GuiVariable> global_variables{};
	void update_variables();
};

struct TabManager {
	std::vector<GeneratorWindowTab> tabs;
	int next_tab_id{};
	inline int add_tab() {
		tabs.push_back(next_tab_id++);
		return next_tab_id - 1;
	}
  inline void remove_tab(int position) {
    if (tabs.empty()) { return; }
		tabs.erase(tabs.begin() + position);
  }
};

struct BuilderWindowTab {
	int id{};
	BuilderWindowTab(const int id_) :id{id_} {}
};

struct TabManagerB {
	std::vector<BuilderWindowTab> tabs;
	int next_tab_id{};
	inline int add_tab() {
		tabs.push_back(next_tab_id++);
		return next_tab_id - 1;
	}
  inline void remove_tab(int position) {
    if (tabs.empty()) { return; }
		tabs.erase(tabs.begin() + position);
  }
};

struct PlantWindowTab {
	int id{};
	PlantWindowTab(const int id_) :id{id_} {}
};

struct TabManagerP {
	std::vector<PlantWindowTab> tabs;
	int next_tab_id{};
	inline int add_tab() {
		tabs.push_back(next_tab_id++);
		return next_tab_id - 1;
	}
  inline void remove_tab(int position) {
    if (tabs.empty()) { return; }
		tabs.erase(tabs.begin() + position);
  }
};

State update_generator_window_tab(
		lsystem_new::LsystemManager& lsystem_manager, GeneratorWindowTab& tab) {
	lsystem_new::Generator* generator = lsystem_manager.get_generator(tab.id);

	if (ImGui::Button("Expand L-String")) {
		generator->iterations++;
		generator->reset_needed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Regenerate L-String")) {
		generator->reset_needed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear L-String")) {
		generator->clear();
		generator->reset_needed = true;
	}
	ImGui::Text("Current iteration: %d", generator->current_iteration);
	ImGui::SameLine();
	if (ImGui::Button("Print L-string")) {
		fmt::print("L-string: {}\n", generator->lstring);
	}


	// ___AXIOM___
	// TODO
	if (ImGui::TreeNode("Axiom")) {
		if (ImGui::InputText("text", generator->axiom, app::gui.textfield_size)) {
			if (lstring_live_regen) {
				generator->reset_needed= true;
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::Button("Add Production")) {
		generator->add_production();
	}
	if (ImGui::Button("Remove Production")) {
		generator->remove_production();
	}

	// ---- Productions ----
	for (int i = 0; i < generator->productions.size(); i++) {
		std::string label = fmt::format("Production {}", i + 1);
		auto &production = generator->productions[i];

		// ---- symbol selection ---- 
		if (ImGui::TreeNode(label.c_str())) {
			static ImGuiComboFlags flags = 0;
			std::string symbol_str = fmt::format("{}", production.symbol);
			const char *combo_preview_value = symbol_str.c_str();
			if (ImGui::BeginCombo("symbol", combo_preview_value, flags)) {
				for (int n = 0; n < lsystem_new::symbols.size(); n++) {
					char symbol = lsystem_new::symbols[n];
					const bool is_selected = (symbol == production.symbol);

					symbol_str = fmt::format("{}", symbol);
					if (ImGui::Selectable(symbol_str.c_str(), is_selected))
						production.symbol = symbol;

					// Set the initial focus when opening the combo (scrolling +
					// keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// ---- condition ---- 
			if (ImGui::InputText("condition", production.condition,
											 app::gui.textfield_size)) {
				generator->reset_needed = true;
			}

			// ---- rule ----
			if (ImGui::InputText("rule", production.rule, 
											 app::gui.textfield_size)) {
				generator->reset_needed = true;
			}
			ImGui::TreePop();
		}
	}


	ImGui::SliderInt("Iterations", &generator->iterations, 1, 16);

	// ---- default variables ----
	for (auto& [symbol_category, value] : generator->symbol_defaults) {
		std::string name = lsystem_new::to_string(symbol_category);
		auto& variable = tab.symbol_defaults[symbol_category]; // var_data

		float temp_value{static_cast<float>(value)};
		if (symbol_category == lsystem_new::SymbolCategory::Rotate) {
			temp_value = value / gk::pi;
		}

		if (ImGui::Checkbox(fmt::format("{} use slider?", name).c_str(),
					&variable.use_slider)) {}
		if (variable.use_slider) {
			ImGui::InputFloat(fmt::format("{}_min", name).c_str(),
					&(variable.slider_start));
			ImGui::InputFloat(fmt::format("{}_max", name).c_str(),
					&(variable.slider_end));
			if (ImGui::SliderFloat(fmt::format("{}_slider", name).c_str(), &(temp_value),
					variable.slider_start, variable.slider_end)) {
				if (symbol_category == lsystem_new::SymbolCategory::Rotate) {
					value = temp_value * gk::pi;
				} else {
					value = temp_value;
				}
				generator->reset_needed = true;
			}
		} else {
			if (ImGui::InputText(fmt::format("{}_expr", name).c_str(),
						variable.expression, app::gui.textfield_size)) {
				auto locals = std::unordered_map<std::string, double>{};
				auto result = lsystem_new::evaluate_expression(locals, generator->global_variables,
						variable.expression);
				if (!result) { 
					// TODO: draw in red
					continue;
				}
				value = result.value();
				generator->reset_needed = true;
			}
		}
	}


	// ---- global variables ----
	for (auto& [name, value] : generator->global_variables) {
		auto& variable = tab.global_variables[name]; // var_data

		if (ImGui::Checkbox(fmt::format("{} use slider?", name).c_str(),
					&variable.use_slider)) {}
		if (variable.use_slider) {
			ImGui::InputFloat(fmt::format("{}_min", name).c_str(),
					&(variable.slider_start));
			ImGui::InputFloat(fmt::format("{}_max", name).c_str(),
					&(variable.slider_end));
			float temp_value{static_cast<float>(value)};
			if (ImGui::SliderFloat(fmt::format("{}_slider", name).c_str(), &(temp_value),
					variable.slider_start, variable.slider_end)) {
				value = temp_value;
				generator->reset_needed = true;
			}
		} else {
			if (ImGui::InputText(fmt::format("{}_expr", name).c_str(),
						variable.expression, app::gui.textfield_size)) {
				auto locals = std::unordered_map<std::string, double>{};
				auto result = lsystem_new::evaluate_expression(locals, generator->global_variables,
						variable.expression);
				if (!result) { 
					// TODO: draw in red
					continue;
				}
				value = result.value();
				generator->reset_needed = true;
			}
		}
	}

	return State::True;
}


State update_generator_window(lsystem_new::LsystemManager& lsystem_manager) {
	// ___MENU_BAR___
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Close", "Ctrl+W")) {
				app::gui.show_demo_window = false;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	static TabManager generator_tabs{};

	// this will be moved to constructor
	// if (gui_module_spec.wait_for_coordinates) {
	// 	if (app::input.mouse_left.just_pressed()) {
	// 		gui_module_spec.wait_for_coordinates = false;
	// 		int current_module_id = lsystem_manager.add_module(app::input.mouse, gk::pi / 2);
	// 		gui_module_spec.tabs.push_back( ModuleTab{gui_module_spec.next_tab_id++, current_module_id} );
	// 		fmt::print("current module count: {}\n", lsystem_manager.modules.size());
	// 	} else {
	// 	}
	// }

	if (ImGui::Button("test")) {
		auto start = util::Clock::now();
		State state{};
		// State state = test_evaluate_expression();
		if (state == State::False) {
			fmt::print("Test False\n");
		} else {
			fmt::print("Test True\n");
		}
		util::ms elapsed = util::Clock::now() - start;
		fmt::print("time: {}\n", elapsed.count());
	}


	static ImGuiTabBarFlags tab_bar_flags =
			ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable |
			ImGuiTabBarFlags_FittingPolicyShrink;

	if (ImGui::BeginTabBar("Generators", tab_bar_flags)) {

		// ---- add generator ----
		if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing |
																			ImGuiTabItemFlags_NoTooltip)) {
			int new_tab_id = generator_tabs.add_tab();
			lsystem_manager.add_generator(new_tab_id);
			lsystem_new::Generator* generator = lsystem_manager.get_generator(new_tab_id);
			GeneratorWindowTab& tab = generator_tabs.tabs[new_tab_id];
			for (auto& [category, value] : generator->symbol_defaults) {
				// double slider_end{};
				// if (category == lsystem_new::SymbolCategory::Rotate) {
				// 	slider_end = 2 * gk::pi;
				// }
				tab.symbol_defaults[category] = GuiVariable{};
			}
			for (auto& [name, value] : generator->global_variables) {
				tab.global_variables[name] = GuiVariable{};
			}
		}

		// Update opened tabs
		for (int i = 0; i < generator_tabs.tabs.size();) {
			bool open = true;
			auto& tab = generator_tabs.tabs[i];
			int tab_id = generator_tabs.tabs[i].id;
			std::string name = fmt::format("{}", tab_id);

			if (ImGui::BeginTabItem(name.c_str(), &open, ImGuiTabItemFlags_None)) {
				State state = update_generator_window_tab(lsystem_manager, tab);
				if (state == State::Error) { return state; }
				ImGui::EndTabItem();
			}

			if (!open) {
				State state = lsystem_manager.remove_generator(tab_id);
				if (state == State::Error) { return state; }
				generator_tabs.remove_tab(i);
			} else {
				i++;
			}
		}

		ImGui::EndTabBar();
	}
	return State::True;
}

State update_plant_window_tab(lsystem_new::LsystemManager& lsystem_manager, 
		PlantWindowTab& tab) {
	ImGui::Text("I am a plant");
	return State::True;
}

State update_builder_window_tab(lsystem_new::LsystemManager& lsystem_manager,
		BuilderWindowTab& tab) {
	lsystem_new::PlantBuilder* plant_builder = lsystem_manager.get_plant_builder(tab.id);
	ImGui::Text("Plants: %d", static_cast<int>(plant_builder->plants.size()));
	static TabManagerP plant_tabs{};

	static ImGuiTabBarFlags tab_bar_flags =
			ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable |
			ImGuiTabBarFlags_FittingPolicyShrink;

	if (ImGui::BeginTabBar("Plants", tab_bar_flags)) {
		if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing |
																			ImGuiTabItemFlags_NoTooltip)) {
			int new_tab_id = plant_tabs.add_tab();
			lsystem_manager.add_plant(plant_builder, new_tab_id);
		}

		// Update opened tabs
		for (int i = 0; i < plant_tabs.tabs.size();) {
			bool open = true;
			int tab_id = plant_tabs.tabs[i].id;
			auto& tab = plant_tabs.tabs[i];
			std::string name = fmt::format("{}", tab_id);

			if (ImGui::BeginTabItem(name.c_str(), &open, ImGuiTabItemFlags_None)) {
				State state = update_plant_window_tab(lsystem_manager, tab);
				if (state == State::Error) { return state; }
				ImGui::EndTabItem();
			}

			if (!open) {
				State state = lsystem_manager.remove_plant(plant_builder, tab_id);
				if (state == State::Error) { return state; }
				plant_tabs.tabs.erase(plant_tabs.tabs.begin() + i);
			} else {
				i++;
			}
		}

		ImGui::EndTabBar();
	}

	return State::True;
}

State update_builder_window(lsystem_new::LsystemManager& lsystem_manager) {
	static TabManagerB builder_tabs{};
	static ImGuiTabBarFlags tab_bar_flags =
			ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable |
			ImGuiTabBarFlags_FittingPolicyShrink;

	if (ImGui::BeginTabBar("Builders", tab_bar_flags)) {
		if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing |
																			ImGuiTabItemFlags_NoTooltip)) {
			int new_tab_id = builder_tabs.add_tab();
			lsystem_manager.add_plant_builder(new_tab_id);
		}

		// Update opened tabs
		for (int i = 0; i < builder_tabs.tabs.size();) {
			bool open = true;
			int tab_id = builder_tabs.tabs[i].id;
			auto& tab = builder_tabs.tabs[i];
			std::string name = fmt::format("{}", tab_id);

			if (ImGui::BeginTabItem(name.c_str(), &open, ImGuiTabItemFlags_None)) {
				State state = update_builder_window_tab(lsystem_manager, tab);
				if (state == State::Error) { return state; }
				ImGui::EndTabItem();
			}

			if (!open) {
				State state = lsystem_manager.remove_plant_builder(tab_id);
				if (state == State::Error) { return state; }
				builder_tabs.tabs.erase(builder_tabs.tabs.begin() + i);
			} else {
				i++;
			}
		}

		ImGui::EndTabBar();
	}
	return State::True;

}

State update_gui(lsystem_new::LsystemManager& lsystem_manager) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // demo_window
  if (app::gui.show_demo_window) {
    ImGui::ShowDemoWindow(&app::gui.show_demo_window);
  }

 	// ___LSYSTEM_MAIN___
  if (app::gui.show_generator_window) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Generators", &app::gui.show_generator_window,
                     ImGuiWindowFlags_MenuBar)) {
			State state = update_generator_window(lsystem_manager);
			if (state == State::Error) { return state; }
    }
    ImGui::End();
  }

	if (app::gui.show_builder_window) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Builders", &app::gui.show_builder_window,
                     ImGuiWindowFlags_MenuBar)) {
			State state = update_builder_window(lsystem_manager);
			if (state == State::Error) { return state; }
    }
    ImGui::End();
	}

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

