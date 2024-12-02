#include "vNesC.h"

#include <SDL2/SDL.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
static int quit_loop = 0;
#endif

#if defined(REPAINT_HOOK)
#define VIEWPORT_INTERVAL 32
#else
#define VIEWPORT_INTERVAL 120

static Uint32 aa = 0, bb = 0, delta = 0;
#endif

static SDL_Surface *surface;
static SDL_Surface *video;
static SDL_Renderer *render;
static SDL_Texture *texture;

static char romname[256];

static void sdl_handle_events(void) {
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
					case SDLK_q:     keyPressed(64); break;
					case SDLK_w:     keyPressed(65); break;
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
					case SDLK_q:     keyReleased(64); break;
					case SDLK_w:     keyReleased(65); break;
				}
				break;
		}
	}

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);
	SDL_RenderCopy(render, texture, NULL, NULL);
	SDL_RenderPresent(render);
}

void repaint() {
#if defined(REPAINT_HOOK)
	memcpy(surface->pixels, screens, screen_length*2);
	sdl_handle_events();
	SDL_Delay(VIEWPORT_INTERVAL);
#endif
}

static void main_loop_step(void) {
#if defined(REPAINT_HOOK)
	run_step();
#else
	aa = SDL_GetTicks();
	delta = aa - bb;

	if (delta > 1000 / VIEWPORT_INTERVAL) {
		//		fprintf(stderr, "fps: %d\n", 1000 / delta);

		bb = aa;

		run_step();
		memcpy(surface->pixels, screens, screen_length*2);
		sdl_handle_events();
	}
#endif
}

int main(int argc, char *argv[]) {
	getWidth      = VNES_VIEWPORT_WIDTH;
	getHeight     = VNES_VIEWPORT_HEIGHT;
	screen_length = getWidth * getHeight;

	int i;
	if(argc < 2){
		printf("No NES ROM!\n");
		return 1;
	}
	strncpy(romname, argv[1], 255);
	initnul();
	if((i=loadrom(*++argv))) {
		printf("Error %d\n",i);
		return i;
	}

	SDL_Window *window = SDL_CreateWindow(
		"vNesC",
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
	//	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0x7C00, 0x03E0, 0x001F, 0x0000);
	//	RGB565
	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
	//	BGR555
	//	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0x001F, 0x03E0, 0x7C00, 0x0000);
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

	init_values();

#if !defined(__EMSCRIPTEN__)
	while (!quit_loop) {
		main_loop_step();
	}
#else
	emscripten_set_main_loop(main_loop_step, VIEWPORT_INTERVAL, 1); // 35 FPS.
#endif

	freeall();

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}

#if defined(SAVE_LOAD)
static FILE* stream;
static int streammode;

static void flush() {
	fwrite(sbuf,ebuf-sbuf, 1, stream);
	ebuf=sbuf;
}

void bytearrayoutputstreamwrite(int byt) {
	if(ebuf>=sbuf+sbufsize) flush();
	*ebuf++=byt;
}

int bytearrayinputstreamread() {
	if(ebuf>=sbuf+sbufsize){
		if (fread(sbuf,sbufsize, 1, stream) > 0)
			ebuf=sbuf;
		else
			return 0;
	}
	return *ebuf++;
}

FILE *openstream(int savemode) {
	char savename[256 + 4]; // ROM name + .sav
	streammode = savemode;

	strncpy(savename, romname, 256);
	strncat(savename, ".sav", 256);

	if(savemode){ //save
		fprintf(stderr, "Saving NES state...\n");
		if((stream=fopen(savename, "wb"))==NULL) return NULL;
		ebuf=sbuf;
	} else{
		fprintf(stderr, "Loading NES state...\n");
		ebuf=sbuf+sbufsize;
		if((stream=fopen(savename, "rb"))==NULL) return NULL;
	}

	fprintf(stderr, "Done!\n");

	return stream;
}

void closestream() {
	if(streammode)
		flush();
	fclose(stream);
}
#endif

char *loadfile(char *s, int *loadfilesize_arg) {
	int loadfilesize;
	*loadfilesize_arg = 0;
	FILE *file = fopen(s, "rb");
	if (file == NULL) {
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	loadfilesize = ftell(file);
	rewind(file);

	char *file_data = (char*)MEM_Alloc_HUGE(loadfilesize);
	if (file_data == NULL) {
		perror("Error allocating memory");
		fclose(file);
		return NULL;
	}

	size_t result = fread(file_data, 1, loadfilesize, file);
	if (result != loadfilesize) {
		MEM_Free_HUGE(file_data);
		fclose(file);
		return NULL;
	}
	fclose(file);

	*loadfilesize_arg = loadfilesize;

	return file_data;
}

void Systemarraycopy(void *from, int foff, void *to, int toff, int size) {
	memmove((char*)to+toff,(char*)from+foff, size);
//	memcpy((char*)to+toff,(char*)from+foff, size);
}

void *MEM_Alloc_HUGE(int mem_size) {
	return malloc(mem_size);
}

void MEM_Free_HUGE(void *ptr) {
	mfree(ptr);
}
