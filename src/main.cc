#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "modules.hpp"
#include <lo/lo.h>


void process_events();
void lock_frame_buf();
bool update_gui(Modules &modules);
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

// a -> a(x, y, l)
// a(x, y, l) -> a((x/l)/l, y/l*2; l-1)[b]
// a<sin(l)>

// store lines in main? vector<line>
int main(int argc, char *argv[]) {
	app::init(960, 540);

	app::gui.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	app::gui.show_window_a = true;
	app::gui.show_window_b = true;
	app::gui.show_window_c = true;

	Modules modules;
	Lsystem &lsystem = modules.lsystem;

	int accum = 0;
	while(app::context.keep_running) {
		process_events();
		lock_frame_buf();

		// calculate axiom and rule lstrings
		lsystem.axiom.plant = lsystem::generate_plant(modules.lsystem, Vec2{(double)app::video.width * 0.3, (double)app::video.height - 100.0},
				lsystem.axiom.text);

		// calculate the lstring, every 30 frames
		if (accum == 0) {
			// the following should instead trigger if some parameter text changes
			// do something with the result?
			if (!lsystem::eval_parameters(lsystem)) {
				puts("eval failed for some parameter");
			}

			lsystem.lstring = lsystem::generate_lstring(modules.lsystem);
			lsystem.plant = lsystem::generate_plant(modules.lsystem, Vec2{(double)app::video.width * 0.7, (double)app::video.height - 100.0},
					lsystem.lstring);
		}

		if (lsystem.live) {
			for (auto &branch : lsystem.plant.branches) {
				draw::wide_line(draw::FrameBuf{(uint32_t*)app::video.frame_buf,
						app::video.width, app::video.height}, 
						Line2{*branch.n1, *branch.n2}, color::fg, lsystem.standard_wd);
			}
		}


		// draw::thick_line({{200, 400}, {600, 700}}, color::fg, 40.0);
		// draw::thick_line_mesh({{200, 400}, app::input.mouse}, color::fg, 40.0);

		if (!update_gui(modules)) {
			return 1;
		}

		// where to put this?
		// -> if strg-down on mouse down save mouse coords as old offset cords
		// -> every frame calculate offset with old mouse coords to new mouse
		// position
		if (!viewport::update_vars()) {
			// puts("wtf?");
		}
		fmt::print("offset: {},{}\n", viewport::vars.xy_offset.x, viewport::vars.xy_offset.y);
		if (viewport::vars.panning_active) {
			puts("pan active");
		}

		// update viewportx§
		render();
		accum++;
		accum %= 60;
	}
	cleanup();
	return 0;
}

// this updates the viewport variables, they are used in the transform funcs
// if strg and mouse click, save cords, then if mouse up while strg, set offset
// per delta, if mouse still down and strg up then reset to saved cords
//
//
// save the old xy offset value
// 


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
bool update_gui(Modules &modules) {
  auto &lsystem = modules.lsystem;

  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // window_a
  if (app::gui.show_window_a) {
    ImGui::ShowDemoWindow(&app::gui.show_window_a);
  }

  // window_b
  if (app::gui.show_window_b) {
    static int counter = 0;

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::Checkbox("Demo Window", &app::gui.show_window_a);
    ImGui::Checkbox("Another Window", &app::gui.show_window_b);

    ImGui::SliderInt("iterations", &modules.lsystem.iterations, 0, 6);
    ImGui::SliderFloat("float", &modules.lsystem.standard_length, 0.0, 100.0);

    if (ImGui::Button("Button"))
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / app::gui.io->Framerate, app::gui.io->Framerate);
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
				for (int i = 0; i < lsystem.n_parameters; i++) {
					std::string label = fmt::format("Parameter {}\n", i);
        	ImGui::InputText(label.c_str(), lsystem.parameter_strings[i], lsystem.text_size);
				}
        ImGui::TreePop();
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

					// TODO i should instead save and restore all text fields in file
					// ___SAVE_FILES_COMBO___
					static std::vector<std::string> save_file_names;
					if (auto result = lsystem::scan_saves()) {
						save_file_names = result.value();
					}

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
								// execute if gui drop down field is selected
                save_fname_index = n;
								if (!lsystem::load_rule_from_file(lsystem.rules[i],
											save_file_names[save_fname_index])) {
									std::puts("fail, rule couldnt be loaded");
								}
							}

              // Set the initial focus when opening the combo (scrolling +
              // keyboard navigation focus)
              if (is_selected)
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
					}

					// ___SYMBOL_SELECTION_COMNO___
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

					// num args
					ImGui::RadioButton("1", &lsystem.rules[i].n_args, 1); ImGui::SameLine();
					ImGui::RadioButton("2", &lsystem.rules[i].n_args, 2); ImGui::SameLine();
					ImGui::RadioButton("3", &lsystem.rules[i].n_args, 3);
					fmt::print("rule{}, args: {}\n", i, lsystem.rules[i].n_args);


					// change color based on conditon value
					bool color_pushed = false;
					if (lsystem.rules[i].condition_state == Lsystem::FIELD_STATE::ERROR) {
						ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
						color_pushed = true;
					} else if (lsystem.rules[i].condition_state == Lsystem::FIELD_STATE::TRUE) {
						color_pushed = true;
						ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.3f, 1.0f, 1.0f));
					}


					// display condition and text
					ImGui::InputText("condition", lsystem.rules[i].condition,
													 lsystem.text_size);
					if (color_pushed) {
						ImGui::PopStyleColor();
					}
          ImGui::InputText("text", lsystem.rules[i].text, lsystem.text_size);

					// a button ?
          if (ImGui::Button("Button")) {
						print_trace();
						return false;
          }

          if (ImGui::Button("Print L-string")) {
						fmt::print("L-string: {}\n", lsystem.lstring);
          }

					ImGui::SliderFloat("wd", &lsystem.standard_wd, 1.0f, 50.0f);

          // ___SAVE_RULE___
					// this works
					static constexpr int text_size = app::Gui::textfield_size;
					static char save_file_name[text_size] = ""; // needs be persistant
          ImGui::InputText("save_file", save_file_name, text_size);
					if (ImGui::Button("Save As")) {
						if (std::strlen(save_file_name) > 0) {
							if (!lsystem::save_rule_as_file(lsystem.rules[i], save_file_name)) {
								puts("Rule saving failed, no file was created!");
							}
						}
					}
          ImGui::TreePop();
        }
      }


			ImGui::Text("nodes: %d", modules.lsystem.plant.node_counter);
			ImGui::Checkbox("live Preview", &lsystem.live);
			if (ImGui::Button("render")) {

				auto t1 = std::chrono::high_resolution_clock::now();
				for (auto &branch : lsystem.plant.branches) {
					draw::wide_line(draw::FrameBuf{(uint32_t*)app::video.frame_buf,
							app::video.width, app::video.height}, 
							Line2{*branch.n1, *branch.n2}, color::fg, lsystem.standard_wd);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> dt_ms = t2 - t1;
				std::cout << "dt_out: " << dt_ms << std::endl;
			}

			// variables
			ImGui::SliderInt("iterations", &modules.lsystem.iterations, 0, 12);
			ImGui::SliderFloat("length", &modules.lsystem.standard_length, 0.0, 200.0);
			ImGui::SliderFloat("angle", &modules.lsystem.standard_angle, 0.0, gk::pi * 2.0);
			ImGui::SliderInt("seg_count", &modules.lsystem.standard_branch_seg_count, 1, 50);


			if (ImGui::Button("send osc")) {
				send();
			}

			// this should not be here as a whole
			if (ImGui::Button("render to images")) {
				int frames = 320;
				int width = 640;
				int height = 480;
				SDL_Surface *frame_surf = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
				for (int i = 0; i < frames; i ++) {
					lsystem.standard_angle -= 0.003;
        	SDL_FillSurfaceRect(frame_surf, nullptr, 0xFF000000);
					lsystem.lstring = lsystem::generate_lstring(modules.lsystem);
					lsystem.plant = lsystem::generate_plant(modules.lsystem,
							Vec2{(double)width * 0.5,
							(double)height * 0.9}, lsystem.lstring);
					for (auto &branch : lsystem.plant.branches) {
						draw::wide_line(draw::FrameBuf{(uint32_t*)frame_surf->pixels,
								frame_surf->w, frame_surf->h}, 
								Line2{*branch.n1, *branch.n2}, 0xFFFF00FF, lsystem.standard_wd);
					}

					fmt::print("{}/frame{:06}\n", app::context.render_path, i);
					std::string frame_file = fmt::format("{}/frame{:06}.png", app::context.render_path, i);
					assert(IMG_SavePNG(frame_surf, frame_file.c_str()));
					// assert(SDL_SaveBMP(frame_surf, frame_file.c_str()));
				}
				SDL_DestroySurface(frame_surf);
			}
		}
    ImGui::End();
  }
	return true;
}

void lock_frame_buf() {
	SDL_SetRenderDrawColor(app::video.renderer, 0, 0, 0, 255);
  SDL_RenderClear(app::video.renderer);

  assert(SDL_LockTexture(app::video.window_texture, NULL,
                         reinterpret_cast<void **>(&app::video.frame_buf),
                         &app::video.pitch));
}

void render() {
  SDL_UnlockTexture(app::video.window_texture);
  SDL_RenderTexture(app::video.renderer, app::video.window_texture, NULL, NULL);
  app::video.frame_buf = nullptr;
  app::video.pitch = 0;


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

