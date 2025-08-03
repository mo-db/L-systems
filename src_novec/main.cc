#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "modules.hpp"

using namespace std;

constexpr double default_distance		= 50;
constexpr double default_angle			= gk::pi/6.2;


string rule_lookup(Lsystem &lsystem, const char c, const double *value) {
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
				x = lsystem.standard_length;
			}

			// define rules here
			// return fmt::format("A({})+A({})--A({})+A({})",
			// 		(x * p), (x * h), (x * h), (x * q));
			return fmt::format("A");
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

string generate_lstr(Lsystem &lsystem) {
	string V = "A,B,a,b,+,[,],(,)";
	string axiom = fmt::format("-A");

	string lstr_expanded;
	string lstr = axiom;
	for (int i = 0; i < lsystem.iterations; i++) {
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
						lstr_expanded += rule_lookup(lsystem, c, &value);
						cnt++;
						// no () follows
					} else {
						lstr_expanded += rule_lookup(lsystem, c, nullptr);
						cnt++;
					}
				// end of string follows
				} else {
					lstr_expanded += rule_lookup(lsystem, c, nullptr);
					cnt++;
				}
			}
		}
		lstr = lstr_expanded;
	}

	return lstr;
}

// store lines in main? vector<line>
int main() {
	App app;
	app::init(app, 960, 540);

	app.gui.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	app.gui.show_window_a = false;
	app.gui.show_window_b = false;
	app.gui.show_window_c = true;

	Modules modules;

	while(app.context.keep_running) {
		app::process_events(app);
		app::lock_frame_buf(app);

		// now the actuall app
		Plant &axiom = modules.lsystem.axiom.plant;
		std::string axiom_str = "A[+A]A[-A]A";

		axiom = lsystem::generate_plant(modules.lsystem, Vec2{(double)app.video.width / 3.0, (double)app.video.height - 100.0},
				axiom_str);

		// string result = generate_lstr(modules.lsystem);
		// process_lstring(result, turtle, tstack, plant, modules.lsystem);


		fmt::print("branches: {}\n", (int)axiom.branches.size());
		for (auto &branch : axiom.branches) {
			draw::line(app, {*(branch.n1), *(branch.n2)}, color::fg, 0.0);
		}

		app::update_gui(app, modules);
		app::render(app);
	}
	app::cleanup(app);
	return 0;
}
