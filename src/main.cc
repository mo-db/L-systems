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
		fmt::print("plant x: {}\n", plant[(int)plant.size()-1].p1.x);
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
					fmt::print("{}\n", c);
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
		cout << "result: " << result << endl;
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

		SDL_UnlockTexture(app.video.window_texture);
		window.clear();
		SDL_RenderTexture(app.video.renderer, app.video.window_texture, NULL, NULL);
		SDL_RenderPresent(app.video.renderer);

		SDL_Delay(1);
	}
}
