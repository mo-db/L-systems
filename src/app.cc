#include "app.hpp"

namespace app {
void init(App &app, int width, int height) {
	app.video.window = NULL;
	app.video.renderer = NULL;
	assert(SDL_Init(SDL_INIT_VIDEO));
  assert(SDL_CreateWindowAndRenderer("Main",
				width, height, SDL_WINDOW_HIGH_PIXEL_DENSITY |
				SDL_WINDOW_MOUSE_CAPTURE, &app.video.window, &app.video.renderer));

	app.video.width = width * SDL_GetWindowPixelDensity(app.video.window);
	app.video.height = height * SDL_GetWindowPixelDensity(app.video.window);

	// texture create with pixels and not window size . retina display scaling
  app.video.window_texture = SDL_CreateTexture(
			app.video.renderer, SDL_PIXELFORMAT_XRGB8888,
			SDL_TEXTUREACCESS_STREAMING, 
			app.video.width, app.video.height);
	assert(app.video.window_texture);

  app.video.density = SDL_GetWindowPixelDensity(app.video.window);
}


// could return vector of key presses, maybe pairs of key and state
void process_events(App &app) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {

    case SDL_EVENT_QUIT:
      app.context.keep_running = false;
      break;

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
      app.input.mouse.x = round(event.motion.x * app.video.density);
      app.input.mouse.y = round(event.motion.y * app.video.density);
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
