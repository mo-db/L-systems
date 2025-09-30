#include "core.hpp"
// #include <iostream>
// #include <SDL3/SDL.h>

SDL_Window *window = nullptr;
SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE |
                               SDL_WINDOW_HIGH_PIXEL_DENSITY |
                               SDL_WINDOW_MOUSE_CAPTURE;
SDL_Renderer *renderer = nullptr;
SDL_Texture *static_texture = nullptr;
SDL_Texture *buffer_texture = nullptr;
uint32_t *buffer_texture_pixels = nullptr;

int pitch = 0;

constexpr int width = 1920;
constexpr int height = 1080;

uint32_t stp_ary[width * height];
uint32_t *static_texture_pixels = stp_ary;

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		print_info("ERROR: SDL_Init()");
		return 1;
	}
  if (!SDL_CreateWindowAndRenderer(
      "main", (width), (height),
      window_flags, &window, &renderer)) {
		print_info("ERROR: SDL_CreateWindowAndRenderer()");
		return 1;
	}
  SDL_SetRenderVSync(renderer, 1);
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  buffer_texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
      width, height);
  if (!buffer_texture) { 
		print_info("ERROR: SDL_CreateTexture()");
		return 1; 
	}
	SDL_SetTextureBlendMode(buffer_texture, SDL_BLENDMODE_NONE);
  static_texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
      width, height);
  if (!buffer_texture) { 
		print_info("ERROR: SDL_CreateTexture()");
		return 1; 
	}
	SDL_SetTextureBlendMode(static_texture, SDL_BLENDMODE_NONE);


	// test 01
	assert(SDL_LockTexture(
			buffer_texture, NULL,
			reinterpret_cast<void **>(&buffer_texture_pixels), &pitch));
	for (int i = 0; i < width * height; i++) {
		buffer_texture_pixels[i] = color::blue;
	}
	SDL_UnlockTexture(buffer_texture);



  bool keep_running = true;
	while (keep_running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				keep_running = false;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  	SDL_RenderClear(renderer);
		//
		// // test 02

		assert(SDL_LockTexture(buffer_texture, NULL,
														reinterpret_cast<void **>(&buffer_texture_pixels),
														&pitch));
		// static int accum = 0;
		// if (++accum >= 60 * 4) {
		// 	accum = 0;
		// 	for (int i = 0; i < width * height; i++) {
		// 		buffer_texture_pixels[i] = 0xFF00FF00;
		// 	}
		// }

		for (int i = 0; i < width * height; i++) {
			buffer_texture_pixels[i] = 0xFF00FF00;
		}
		SDL_UnlockTexture(buffer_texture);

		SDL_RenderTexture(renderer, buffer_texture, NULL, NULL);

		static int accum = 0;
		if (++accum >= 60 * 1) {
			accum = 0;
			auto start = util::Clock::now();
			for (int i = 0; i < width * height; i++) {
				static_texture_pixels[i] = 0xFF00FF00;
			}
			util::ms elapsed = util::Clock::now() - start;
			print_info(fmt::format("fill pixels: {}\n", elapsed.count()));


			start = util::Clock::now();
			SDL_UpdateTexture(static_texture, nullptr, static_texture_pixels, width * 4);
			elapsed = util::Clock::now() - start;
			print_info(fmt::format("update texture: {}\n", elapsed.count()));

			start = util::Clock::now();
			SDL_RenderTexture(renderer, static_texture, NULL, NULL);
			elapsed = util::Clock::now() - start;
			print_info(fmt::format("render texture: {}\n", elapsed.count()));
		}
  	// util::ms frame_time = util::ms(1000.0 / 60.0);


  	SDL_RenderTexture(renderer, static_texture, NULL, NULL);
		SDL_FRect rect;
		rect.x = width / 2.0f - 50.0f;  // center x
		rect.y = height / 2.0f - 50.0f; // center y
		rect.w = 100.0f;
		rect.h = 100.0f;
		SDL_RenderRect(renderer, &rect);
  	SDL_RenderPresent(renderer);
	}
	return 0;
}
