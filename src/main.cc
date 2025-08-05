#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"
#include "modules.hpp"

// store lines in main? vector<line>
int main() {
	App app;
	app::init(app, 960, 540);

	app.gui.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	app.gui.show_window_a = true;
	app.gui.show_window_b = true;
	app.gui.show_window_c = true;

	Modules modules;
	Lsystem &lsystem = modules.lsystem;

	std::string complete_lstring = "";
	Plant complete_plant{};

	int accum = 0;
	while(app.context.keep_running) {
		app::process_events(app);
		app::lock_frame_buf(app);

		// calculate axiom and rule lstrings
		Plant axiom_plant = lsystem::generate_plant(modules.lsystem, Vec2{(double)app.video.width * 0.3, (double)app.video.height - 100.0},
				lsystem.axiom.text);

		// calculate the lstring, every 30 frames
		if (accum == 0) {
			complete_lstring = lsystem::generate_lstring(modules.lsystem);
			complete_plant = lsystem::generate_plant(modules.lsystem, Vec2{(double)app.video.width * 0.7, (double)app.video.height - 100.0},
					complete_lstring);
		}

		// draw the plants
		for (auto &branch : axiom_plant.branches) {
			draw::line(app, {*(branch.n1), *(branch.n2)}, color::fg, 0.0);
		}
		for (auto &branch : complete_plant.branches) {
			draw::line(app, {*(branch.n1), *(branch.n2)}, color::fg, 0.0);
		}

		app::update_gui(app, modules);
		app::render(app);
		accum++;
		accum %= 60;
	}
	app::cleanup(app);
	return 0;
}
