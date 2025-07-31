#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "turtle.hpp"

using namespace std;

Vec2 rotate(Vec2 &v, double angle) {
	Vec2 a;
	a.x = v.x * cos(angle) - v.y * -sin(angle);
	a.y = v.x * -sin(angle) + v.y * cos(angle);
	return a;
}

void turtle_action(Turtle &turtle, vector<Turtle> &tstack, char c,
									 double value, vector<Line2> plant) {
  Vec2 p1, p2;
  switch (c) {
  case 'F':
    p1 = {turtle.x, turtle.y};
    turtle::move(turtle, value);
    p2 = {turtle.x, turtle.y};
    plant.push_back(Line2{p1, p2});
    break;
  case 'f':
    turtle::move(turtle, value);
    break;
  case '+':
    turtle::turn(turtle, value);
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

string rule_lookup(const char c, const double *value) {
	double f = 1.0;
	double p = 0.3;
	double q = f - p;
	double h = pow((p * q), 0.5);

	double x;
	double default_distance		= 50.0;
	double default_angle			= gk::pi/3;

	switch (c) {
		case 'A':
			if (value) {
				x = *value;	
			} else {
				x = default_distance;
			}

			// define rules here
			return fmt::format("A({})+A({})--A({})+A({})",
					(x * p), (x * h), (x * h), (x * q));


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
		// case '+':
		// 	break;
		// case '-':
		// 	break;

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
	string axiom = "A(1)";

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

// store lines in main? vector<line>
int main() {
	App app;
	app::init(app, 1920/2, 1080/2);
	Frame window;
	assert(SDL_LockTexture(app.video.window_texture, NULL, 
				 reinterpret_cast<void **>(&window.buf), &window.pitch));
	std::fill_n(window.buf, app.video.width * app.video.height, color::bg);

	vector<Line2> plant;
	vector<Turtle> tstack;

	string result = generate_lstr(2);
	cout << "result: " << result << endl;


	SDL_UnlockTexture(app.video.window_texture);
	window.clear();
	SDL_RenderTexture(app.video.renderer, app.video.window_texture, NULL, NULL);
	SDL_RenderPresent(app.video.renderer);

	while(app.context.keep_running) {
		app::process_events(app);
		SDL_Delay(1);
	}
}
