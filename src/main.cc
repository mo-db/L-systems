#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "turtle.hpp"
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
				x = lsystem.default_distance;
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

void turtle_action(Turtle &turtle, vector<Turtle> &tstack, vector<Line2> &plant,
		const char c, const double *value, Lsystem &lsystem) {

	double x;

  Vec2 p1, p2;
  switch (c) {

	// move and add branch to plant
  case 'F':
		if (value) {
			x = *value * default_distance;	
		} else {
			x = lsystem.default_distance;
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
			x = lsystem.default_distance;
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

void process_lstring(string lstr, Turtle &turtle, vector<Turtle> &tstack, vector<Line2> &plant, Lsystem &lsystem) {
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
			turtle_action(turtle, tstack, plant,  c, nullptr, lsystem);
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
					turtle_action(turtle, tstack, plant,  c, &value, lsystem);
					cnt++;
					// no () follows
				} else {
					turtle_action(turtle, tstack, plant,  c, nullptr, lsystem);
					cnt++;
				}
			// end of string follows
			} else {
				turtle_action(turtle, tstack, plant,  c, nullptr, lsystem);
				cnt++;
			}
		}
	}

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
		Turtle turtle{round(app.video.width)/2, round(app.video.height), gk::pi/2};
		vector<Line2> plant;
		vector<Turtle> tstack;

		string result = generate_lstr(modules.lsystem);
		process_lstring(result, turtle, tstack, plant, modules.lsystem);
		for (auto &branch : plant) {
			draw::line(app, branch, color::fg, 1.0);
		}

		app::update_gui(app, modules);
		app::render(app);
	}
	app::cleanup(app);
	return 0;
}
