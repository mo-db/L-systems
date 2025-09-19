#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "lsystem.hpp"
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

	lm::plant.init(Vec2{(double)app::video.width/2, app::video.height - 50.0}, gk::pi / 2);
	fmt::print("bytes: {}\n", lm::plant.max_nodes * sizeof(Vec2));

	app::gui.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	app::gui.show_window_a = true;
	app::gui.show_window_b = true;
	app::gui.show_window_c = true;
	app::gui.show_rendering_window = true;
	app::gui.show_lsystem_window = true;

	draw::FrameBuf fb_main{app::video.window_texture_pixels, app::video.width, app::video.height};

	while(app::context.keep_running) {
		app::context.frame_start = util::Clock::now();
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
			lm::plant.needs_regen = true;
			puts("pan active");
		}

		// ---- update the global and default variables -> on gui event? ----
		lm::update_vars();

		// ---- generate plant over one or more frames ----
		if (lm::plant.needs_regen || lm::plant.regenerating) {
			if (lm::plant.needs_regen) {
				print_info("clear plant");
				lm::plant.needs_regen = false;
				lm::plant.needs_redraw = true;
				lm::plant.current_lstring_index = 0;
				lm::plant.clear();
			}
			bool done = generate_plant_timed(lm::system.lstring, lm::plant, lm::plant.current_lstring_index);
			if (done) {
				lm::plant.regenerating = false;
			} else {
				done = generate_plant_timed(lm::system.lstring, lm::plant, lm::plant.current_lstring_index);
				lm::plant.regenerating = true;
			}
			lm::plant.redrawing = true; // plant_check if redraw
		}

		// ---- draw plant over one or more frames ----
		if (lm::plant.needs_redraw || lm::plant.redrawing) {
			// how often to clear and the opacity of the plant should be a setting
			if (lm::plant.needs_redraw) {
				print_info("clear texture");
				lm::plant.needs_redraw = false;
				lm::plant.current_branch = 0;
				draw::clear(fb_main, color::bg);
			}
			bool done = lm::draw_plant_timed(lm::plant, lm::plant.current_branch, color::fg, fb_main);
			if (done) {
				lm::plant.redrawing = false;
			} else {
				lm::plant.redrawing = true;
			}
		}

		// ---- push framebuffer and render gui ----
		render();
	}
	cleanup();
	return 0;
}

// could return vector of key presses, maybe pairs of key and state
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

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:

		case SDL_EVENT_KEY_UP:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					if (!event.key.repeat) {
						app::input.shift_set = false;
					}
					break;
				case SDLK_LCTRL:
					if (!event.key.repeat) {
						app::input.ctrl_set = false;
					}
					break;
				default: break;
			}
			break;

		case SDL_EVENT_KEY_DOWN:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					if (!event.key.repeat) {
						app::input.shift_set = true;
					}
					break;
				case SDLK_LCTRL:
					if (!event.key.repeat) {
						app::input.ctrl_set = true;
					}
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
      app::input.mouse_left_down = event.button.down;
			app::input.mouse_click = true;
      break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      app::input.mouse_left_down = event.button.down;
      break;

		default: break;
		}
	}
}
bool update_gui() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // window_a
  if (app::gui.show_window_a) {
    ImGui::ShowDemoWindow(&app::gui.show_window_a);
  }

	if (app::gui.show_lsystem_window) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("L-System", &app::gui.show_lsystem_window,
                     ImGuiWindowFlags_MenuBar)) {
		}
    ImGui::End();
	}

	if (app::gui.show_rendering_window) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Plant", &app::gui.show_rendering_window,
                     ImGuiWindowFlags_MenuBar)) {
      if (ImGui::TreeNode("Global Variables")) {
				for (int i = 0; i < lm::glob_vars.quant; i++) {
					lm::Var *var = lm::glob_vars.var(i);
        	ImGui::InputText(var->label.c_str(), var->expr, lm::system.text_size);
				}
        ImGui::TreePop();
      }
		}
    ImGui::End();
	}

  // ___LSYSTEM_MAIN___
  static bool open = true;
  bool *p_open = &open;
  if (app::gui.show_window_c) {

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

      // __PARAMETERS___
      if (ImGui::TreeNode("Parameters")) {
				for (int i = 0; i < lm::system.n_parameters; i++) {
					std::string label = fmt::format("Parameter {}\n", i);
        	ImGui::InputText(label.c_str(), lm::system.parameter_strings[i], lm::system.text_size);
				}
        ImGui::TreePop();
      }

			// new part!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (ImGui::Button("Generate L-String")) {
					bool success = lm::system.generate();
			}
			ImGui::SameLine();
			if (ImGui::Button("Expand L-String")) {
					bool success = lm::system.expand();
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear L-String")) {
				lm::system.current_iteration = 0;
				lm::system.lstring.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("Print L-string")) {
				fmt::print("L-string: {}\n", lm::system.lstring);
			}


			if (ImGui::Button("Regen Plant")) {
				lm::plant.needs_regen = true;
			}

      // ___AXIOM___
      if (ImGui::TreeNode("Axiom")) {
        ImGui::InputText("text", lm::system.axiom, app::gui.textfield_size);
        ImGui::TreePop();
      }

			// ___RULES___
      for (int i = 0; i < lm::system.max_rules; i++) {
        std::string label = fmt::format("Rule {}", i + 1);

        if (ImGui::TreeNode(label.c_str())) {


					// ___SYMBOL_SELECTION_COMNO___
          static ImGuiComboFlags flags = 0;
          const char *combo_preview_value =
              lm::system.alphabet[lm::system.rules[i].symbol_index];
          if (ImGui::BeginCombo("symbol", combo_preview_value, flags)) {
            for (int n = 0; n < lm::system.alphabet_size; n++) {
              const bool is_selected = (lm::system.rules[i].symbol_index == n);
              if (ImGui::Selectable(lm::system.alphabet[n], is_selected))
                lm::system.rules[i].symbol_index = n;

              // Set the initial focus when opening the combo (scrolling +
              // keyboard navigation focus)
              if (is_selected)
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
          }

					// num args
					ImGui::RadioButton("1", &lm::system.rules[i].n_args, 1); ImGui::SameLine();
					ImGui::RadioButton("2", &lm::system.rules[i].n_args, 2); ImGui::SameLine();
					ImGui::RadioButton("3", &lm::system.rules[i].n_args, 3);
					// fmt::print("rule{}, args: {}\n", i, lm::system.rules[i].n_args);


					// change color based on conditon value
					bool color_pushed = false;
					if (lm::system.rules[i].condition_state == util::STATE::ERROR) {
						ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
						color_pushed = true;
					} else if (lm::system.rules[i].condition_state == util::STATE::TRUE) {
						color_pushed = true;
						ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.3f, 1.0f, 1.0f));
					}


					// display condition and text
					ImGui::InputText("condition", lm::system.rules[i].condition,
													 lm::system.text_size);
					if (color_pushed) {
						ImGui::PopStyleColor();
					}
          ImGui::InputText("text", lm::system.rules[i].text, lm::system.text_size);

          ImGui::TreePop();
        }
      }

			ImGui::Text("nodes: %d", lm::plant.node_count);

			// ---- standard variables ----
			ImGui::SliderInt("iterations", &lm::system.iterations, 0, 12);
			if (ImGui::SliderFloat("length", &lm::system.standard_length, 0.0, 200.0)) {
				lm::plant.needs_regen = true;
			}
			if (ImGui::SliderFloat("angle", &lm::system.standard_angle, 0.0, gk::pi * 2.0)) {
				lm::plant.needs_regen = true;
			}
			if (ImGui::SliderInt("Width", &lm::system.standard_wd, 1, 20)) {
				lm::plant.needs_regen = true;
			}
			// ImGui::SliderInt("seg_count", &lm::system.standard_branch_seg_count, 1, 50);


			// testing
			static std::string test = "test";
			if (ImGui::CustInputText("testlabel", &test)) {}

			// ---- global variables ----
			for (int i = 0; i < lm::glob_vars.quant; i++) {
				lm::Var *var = lm::glob_vars.var(i);
				if (ImGui::Checkbox(fmt::format("{} use slider?", var->label).c_str(),
							&var->use_slider)) {}
				if (var->use_slider) {
					// ImGui::SameLine();
					ImGui::InputFloat(fmt::format("{}_min", var->label).c_str(),
							&(var->slider_start));
					ImGui::InputFloat(fmt::format("{}_max", var->label).c_str(),
							&(var->slider_end));
					if (ImGui::SliderFloat(fmt::format("{}_slider", var->label).c_str()
								, &(var->value),
							var->slider_start, var->slider_end)) {
						(var->expr)[0] = '\0';
						lm::plant.needs_regen = true;
					}
				} else {
					if (ImGui::InputText(fmt::format("{}_text", var->label).c_str(),
								var->expr, app::gui.textfield_size)) {
						lm::plant.needs_regen = true;
					}
				}
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

