#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "turtle.hpp"

using namespace std;

constexpr double default_distance		= 5;
constexpr double default_angle			= gk::pi/6.2;
double len_count = 0.0;
double new_angle = gk::pi/6.2;
double new_dist = default_distance;

string rule_lookup(const char c, const double *value) {
	double f = 1.0;
	double p = 0.3;
	double q = f - p;
	double h = sqrt(p * q);

	double x;
	switch (c) {
		case 'A':
			if (value) {
				x = *value;	
			} else {
				x = default_distance;
			}

			// define rules here
			// return fmt::format("A({})+A({})--A({})+A({})",
			// 		(x * p), (x * h), (x * h), (x * q));
			return fmt::format("A({0})[+A({0})]A({0})[-A({0})][A({0})]", x);
			// return fmt::format("A[+A]A[-A]A");


			// no rules matched
			if (value) {
				return fmt::format("{}({})", c, x);
			} else {
				return fmt::format("{}", c);
			}
			break;
		case 'B':
			if (value) {
				x = *value;	
			} else {
				x = default_distance;
			}

			// define rules here
			return fmt::format("A+");
			// if (x > 1.0) {
			// 	return fmt::format("B-");
			// }

			// no match
			if (value) {
				return fmt::format("{}({})", c, x);
			} else {
				return fmt::format("{}", c);
			}
			break;
		case '+':
			if (value) {
				x = *value;	
			} else {
				x = default_angle;
			}

			return fmt::format("{}({})", c, x+gk::pi/16 * sin(new_angle));

			// no match
			if (value) {
				return fmt::format("{}({})", c, x);
			} else {
				return fmt::format("{}", c);
			}
			break;
		case '-':
			if (value) {
				x = *value;	
			} else {
				x = default_angle;
			}
			// no match
			if (value) {
				return fmt::format("{}({})", c, x);
			} else {
				return fmt::format("{}", c);
			}
			break;

		// no match
		default: 
			if (value) {
				return fmt::format("{}({})", c, *value);
			} else {
				return fmt::format("{}", c);
			}
			break;
	}
}

string generate_lstr(int iterations) {
	string V = "A,B,a,b,+,[,],(,)";
	string axiom = fmt::format("A[+A]A[-A][A]");

	string lstr_expanded;
	string lstr = axiom;
	for (int i = 0; i < iterations; i++) {
		lstr_expanded = "";
		int cnt = 0;
		while (cnt < (int)lstr.size()) {
			char c = lstr[cnt];
			// if stack symbol -> copy and continue
			if (c == '[' || c == ']') {
				lstr_expanded += c;
				cnt++;
			// else process
			} else {
				// check if not last char
				if (cnt + 1 < (int)lstr.size()) {
					// () follows -> process whole string
					if (lstr[cnt+1] == '(') {
						cnt += 2; // skip bracket
						string value_string = "";
						while (lstr[cnt] != ')') {
							value_string += lstr[cnt++];
						}
						double value = atof(value_string.c_str());
						lstr_expanded += rule_lookup(c, &value);
						cnt++;
						// no () follows
					} else {
						lstr_expanded += rule_lookup(c, nullptr);
						cnt++;
					}
				// end of string follows
				} else {
					lstr_expanded += rule_lookup(c, nullptr);
					cnt++;
				}
			}
		}
		lstr = lstr_expanded;
	}

	return lstr;
}

void turtle_action(Turtle &turtle, vector<Turtle> &tstack, vector<Line2> &plant,
		const char c, const double *value) {

	double x;

  Vec2 p1, p2;
  switch (c) {

	// move and add branch to plant
  case 'F':
		if (value) {
			x = *value * default_distance;	
		} else {
			x = default_distance;
		}

    p1 = {turtle.x, turtle.y};
    turtle::move(turtle, x);
    p2 = {turtle.x, turtle.y};
    plant.push_back(Line2{p1, p2});
    break;

	// move without adding a branch to plant
  case 'f':
		if (value) {
			x = *value;	
		} else {
			x = default_distance;
		}

    turtle::move(turtle, x);
    break;

	// rotate clockwise
  case '+':
		if (value) {
			x = *value;	
		} else {
			x = default_angle;
		}

    turtle::turn(turtle, x);
    break;


	// rotate counter-clockwise
  case '-':
		if (value) {
			x = *value;	
		} else {
			x = default_angle;
		}

    turtle::turn(turtle, -x);
    break;
  case '[':
    tstack.push_back(turtle);
    break;
  case ']':
    turtle = tstack.back();
    tstack.pop_back();
    break;
  default:
    break;
  }
}

void process_lstring(string lstr, Turtle &turtle, vector<Turtle> &tstack, vector<Line2> &plant) {
	int cnt = 0;
	while (cnt < (int)lstr.size()) {
		char c = lstr[cnt];
		// merge edge rewriting
		if (c == 'A' || c == 'B') {
			c = 'F';
		}
		if (c == 'a' || c == 'b') {
			c = 'f';
		}
		// if stack symbol -> copy and continue
		if (c == '[' || c == ']') {
			turtle_action(turtle, tstack, plant,  c, nullptr);
			cnt++;
		// else process
		} else {
			// check if not last char
			if (cnt + 1 < (int)lstr.size()) {
				// () follows -> process whole string
				if (lstr[cnt+1] == '(') {
					cnt += 2; // skip bracket
					string value_string = "";
					while (lstr[cnt] != ')') {
						value_string += lstr[cnt++];
					}
					double value = atof(value_string.c_str());
					turtle_action(turtle, tstack, plant,  c, &value);
					cnt++;
					// no () follows
				} else {
					turtle_action(turtle, tstack, plant,  c, nullptr);
					cnt++;
				}
			// end of string follows
			} else {
				turtle_action(turtle, tstack, plant,  c, nullptr);
				cnt++;
			}
		}
	}

}

// store lines in main? vector<line>
int main() {
	App app;
	app::init(app, 1920/2, 1080/2);
	Frame window;



	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1.0);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	// style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(app.video.window, app.video.renderer);
	ImGui_ImplSDLRenderer3_Init(app.video.renderer);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_demo_window = true;
	bool show_another_window = true;



	while(app.context.keep_running) {
		app::process_events(app);

		assert(SDL_LockTexture(app.video.window_texture, NULL,
					 reinterpret_cast<void **>(&window.buf), &window.pitch));
		window.width = app.video.width;
		window.height = app.video.height;
		std::fill_n(window.buf, app.video.width * app.video.height, color::bg);

		Turtle turtle{round(app.video.width)/2, round(app.video.height), gk::pi/2};
		vector<Line2> plant;
		vector<Turtle> tstack;

		string result = generate_lstr(4);
		process_lstring(result, turtle, tstack, plant);
		for (auto &branch : plant) {
			draw::line(window, branch, color::fg, 1.0);
		}

		new_angle += 0.01;
		len_count += 0.01;
		new_dist = sin(len_count) * default_distance * 2.0;
		if (new_dist < 0.0) {
			new_dist *= -1.0;
		}

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

    ImGui::ShowDemoWindow(&show_demo_window);
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
						counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::End();
		}


		SDL_UnlockTexture(app.video.window_texture);
		window.clear();
		SDL_RenderTexture(app.video.renderer, app.video.window_texture, NULL, NULL);

    ImGui::Render();
		SDL_SetRenderScale(app.video.renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(app.video.renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);


		// SDL_RenderClear(app.video.renderer);
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app.video.renderer);
		SDL_RenderPresent(app.video.renderer);

	}
	// Cleanup
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(app.video.renderer);
	SDL_DestroyWindow(app.video.window);
	SDL_Quit();

	return 0;
}
