#include "vNesC.h"

#include <SDL2/SDL.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
static int quit_loop = 0;
#endif

char romname[256];
volatile int running;

static SDL_Surface *surface;
static SDL_Surface *video;
static SDL_Renderer *render;
static SDL_Texture *texture;

#define VIEWPORT_INTERVAL 35

void repaint() {
	memcpy(surface->pixels, screens, 132*176*2);
}

void check_keys(void) {
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
//	yeti.keyboard.a      = keyboard[SDL_SCANCODE_Z] || keyboard[SDL_SCANCODE_LCTRL] || keyboard[SDL_SCANCODE_RETURN];
//	yeti.keyboard.b      = keyboard[SDL_SCANCODE_X] || keyboard[SDL_SCANCODE_SPACE];
//	yeti.keyboard.select = keyboard[SDL_SCANCODE_ESCAPE];
//	yeti.keyboard.right  = keyboard[SDL_SCANCODE_RIGHT] || keyboard[SDL_SCANCODE_D];
//	yeti.keyboard.left   = keyboard[SDL_SCANCODE_LEFT] || keyboard[SDL_SCANCODE_A];
//	yeti.keyboard.up     = keyboard[SDL_SCANCODE_UP] || keyboard[SDL_SCANCODE_W];
//	yeti.keyboard.down   = keyboard[SDL_SCANCODE_DOWN] || keyboard[SDL_SCANCODE_S];
//	yeti.keyboard.l      = keyboard[SDL_SCANCODE_C] || keyboard[SDL_SCANCODE_PAGEUP];
//	yeti.keyboard.r      = keyboard[SDL_SCANCODE_V] || keyboard[SDL_SCANCODE_PAGEDOWN];
}

void main_loop_step(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
#if !defined(__EMSCRIPTEN__)
			quit_loop = 1;
			exit(1);
#endif
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
				case SDLK_UP:    keyPressed(50); break;
				case SDLK_DOWN:  keyPressed(56); break;
				case SDLK_LEFT:  keyPressed(52); break;
				case SDLK_RIGHT: keyPressed(54); break;
				case SDLK_z:     keyPressed(49); break;
				case SDLK_x:     keyPressed(51); break;
				case SDLK_a:     keyPressed(53); break;
				case SDLK_s:     keyPressed(48); break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
				case SDLK_UP:    keyReleased(50); break;
				case SDLK_DOWN:  keyReleased(56); break;
				case SDLK_LEFT:  keyReleased(52); break;
				case SDLK_RIGHT: keyReleased(54); break;
				case SDLK_z:     keyReleased(49); break;
				case SDLK_x:     keyReleased(51); break;
				case SDLK_a:     keyReleased(53); break;
				case SDLK_s:     keyReleased(48); break;
			}
			break;
		}
	}

	//    static const char *a_java_lang_String_array1d_static_fld[] = {
//        "Up", "Down", "Left", "Right", "A", "B", "Start", "Select", "AA", "BB",
//        "Trigger", "Quick Save", "Quick Load", "FullScreen"
//    };
    static int key[] = {
        50, 56, 52, 54, 49, 51,
        53, 48, 55, 57,
        35, 0, 0, 42
    };

//	check_keys();

//	memcpy(surface->pixels, screens, getWidth*getHeight*2);

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);
	SDL_RenderCopy(render, texture, NULL, NULL);
	SDL_RenderPresent(render);

	SDL_Delay(5);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	int i;
  if(argc<2){
    printf("No ROM!\n");
    return 1;
  }
  initnul();
  h();
  if(i=loadrom(*++argv)){
    printf("Error %d\n",i);
    return i;
  }

	SDL_Window *window = SDL_CreateWindow(
		"Yeti3D",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		getWidth, getHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);
	if (window == NULL) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (render == NULL) {
		SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	video = SDL_CreateRGBSurface(0, getWidth, getHeight, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (video == NULL) {
		SDL_Log("SDL_CreateRGBSurface (video) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

//	RGB555
//	surface = SDL_CreateRGBSurface(0, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0x7C00, 0x03E0, 0x001F, 0x0000);
//	RGB565
	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
//	BGR555
//	surface = SDL_CreateRGBSurface(0, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0x001F, 0x03E0, 0x7C00, 0x0000);
	if (surface == NULL) {
		SDL_Log("SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, getWidth, getHeight);
	if (texture == NULL) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

//#if !defined(__EMSCRIPTEN__)
//	while (!quit_loop) {
//		main_loop_step();
//		SDL_Delay(1000 / VIEWPORT_INTERVAL);
//	}
//#else
//	emscripten_set_main_loop(main_loop_step, VIEWPORT_INTERVAL, 1); // 35 FPS.
//#endif

	run();

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
