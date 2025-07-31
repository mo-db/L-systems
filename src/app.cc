#include "app.hpp"

namespace app {
void init(App &app, int width, int height) {
  assert(SDL_Init(SDL_INIT_VIDEO));
  app.video.window = NULL;
  app.video.renderer = NULL;
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_HIGH_PIXEL_DENSITY |
                                 SDL_WINDOW_MOUSE_CAPTURE;

  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  assert(SDL_CreateWindowAndRenderer(
      "main", (int)(width * main_scale), (int)(height * main_scale),
      window_flags, &app.video.window, &app.video.renderer));
  SDL_SetRenderVSync(app.video.renderer, 1);
  SDL_SetWindowPosition(app.video.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	
	float pixel_density = SDL_GetWindowPixelDensity(app.video.window);
	fmt::print("main scale: {}, pixel density: {}", main_scale, pixel_density);

  app.video.width = width * SDL_GetWindowPixelDensity(app.video.window);
  app.video.height = height * SDL_GetWindowPixelDensity(app.video.window);

  // texture create with pixels and not window size . retina display scaling
  app.video.window_texture = SDL_CreateTexture(
      app.video.renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
      app.video.width, app.video.height);
  assert(app.video.window_texture);

  app.video.density = SDL_GetWindowPixelDensity(app.video.window);
}


// could return vector of key presses, maybe pairs of key and state
void process_events(App &app) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);

		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
				event.window.windowID == SDL_GetWindowID(app.video.window)) {
      app.context.keep_running = false;
		}

    switch (event.type) {
    case SDL_EVENT_QUIT:
      app.context.keep_running = false;
      break;

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:

		case SDL_EVENT_KEY_UP:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					if (!event.key.repeat) {
						app.input.shift_set = false;
					}
					break;
				case SDLK_LCTRL:
					if (!event.key.repeat) {
						app.input.ctrl_set = false;
					}
					break;
				default: break;
			}
			break;

		case SDL_EVENT_KEY_DOWN:
			switch(event.key.key) {
				case SDLK_LSHIFT:
					if (!event.key.repeat) {
						app.input.shift_set = true;
					}
					break;
				case SDLK_LCTRL:
					if (!event.key.repeat) {
						app.input.ctrl_set = true;
					}
					break;
        case SDLK_ESCAPE:
					break;
				default: break;
			}
			break;

     case SDL_EVENT_MOUSE_MOTION:
			// if (!app.gui.io.WantCaptureMouse) {
				// app.input.mouse.x = round(event.motion.x * app.video.density);
				// app.input.mouse.y = round(event.motion.y * app.video.density);
			// }
      break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      app.input.mouse_left_down = event.button.down;
			app.input.mouse_click = true;
      break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      app.input.mouse_left_down = event.button.down;
      break;

		default: break;
		}
	}
}
} // namespace app
