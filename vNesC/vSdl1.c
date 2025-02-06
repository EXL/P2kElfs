// SDL1 port

#include "vNesC.h"

#include <time.h>

#include <SDL/SDL.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
static int quit_loop = 0;
#endif

#if defined(REPAINT_HOOK)
#define VIEWPORT_INTERVAL 1000 / 75
#else
#define VIEWPORT_INTERVAL 120

static Uint32 aa = 0, bb = 0, delta = 0;
#endif

static SDL_Surface *surface;
static SDL_Surface *video;

static char romname[256];

static void save_screenshot(void);
static void save_sdl_screenshot(void);

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
					case SDLK_UP:     keyPressed(50);  break;
					case SDLK_DOWN:   keyPressed(56);  break;
					case SDLK_LEFT:   keyPressed(52);  break;
					case SDLK_RIGHT:  keyPressed(54);  break;
					case SDLK_z:      keyPressed(49);  break;
					case SDLK_x:      keyPressed(51);  break;
					case SDLK_a:      keyPressed(53);  break;
					case SDLK_s:      keyPressed(48);  break;
					case SDLK_q:      keyPressed(64);  break;
					case SDLK_w:      keyPressed(65);  break;
#if !defined(__EMSCRIPTEN__)
					case SDLK_o:   save_sdl_screenshot();  break;
					case SDLK_p:   save_screenshot();  break;
					case SDLK_ESCAPE: quit_loop = 1;   break;
#endif
					default:                           break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					case SDLK_UP:     keyReleased(50); break;
					case SDLK_DOWN:   keyReleased(56); break;
					case SDLK_LEFT:   keyReleased(52); break;
					case SDLK_RIGHT:  keyReleased(54); break;
					case SDLK_z:      keyReleased(49); break;
					case SDLK_x:      keyReleased(51); break;
					case SDLK_a:      keyReleased(53); break;
					case SDLK_s:      keyReleased(48); break;
					case SDLK_q:      keyReleased(64); break;
					case SDLK_w:      keyReleased(65); break;
					default:                           break;
				}
				break;
		}
	}

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_Flip(video);
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
	A = 2; /* Frameskip: 1-9 */
	t_boolean_static_fld = true; /* Optimize: 0-1 */

	int i;
	if(argc < 2){
		fprintf(stderr, "No NES ROM!\n");
		return 1;
	}
	strncpy(romname, argv[1], 255);
	initnul();
	if((i=loadrom(*++argv))) {
		fprintf(stderr, "Error %d\n",i);
		return i;
	}

	video = SDL_SetVideoMode(getWidth, getHeight, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (video == NULL) {
		fprintf(stderr, "SDL_SetVideoMode (video) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_WM_SetCaption("vNesC SDL1", NULL);

	//	RGB555
	//	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0x7C00, 0x03E0, 0x001F, 0x0000);
	//	RGB565
	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
	//	BGR555
	//	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 16, 0x001F, 0x03E0, 0x7C00, 0x0000);
	if (surface == NULL) {
		fprintf(stderr, "SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
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

	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);

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

static void save_screenshot(void) {
	unsigned char bmphead[70] = {
		0x42, 0x4D, 0x46, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00,
		0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xF0, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xE0,
		0x01, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07,
		0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	*(int *)(bmphead+0x02) = VNES_VIEWPORT_WIDTH * VNES_VIEWPORT_HEIGHT * 2 + 70;
	*(int *)(bmphead+0x12) = VNES_VIEWPORT_WIDTH;
	*(int *)(bmphead+0x16) = VNES_VIEWPORT_HEIGHT;
	*(int *)(bmphead+0x22) = VNES_VIEWPORT_WIDTH * VNES_VIEWPORT_HEIGHT * 2;

	char fname[256];
	snprintf(fname, 256, "Screenshot_%ld.bmp", time(NULL));

	FILE *f;
	int i,j;
	short scr[VNES_VIEWPORT_WIDTH * VNES_VIEWPORT_HEIGHT];
	short *s=(short *)screens;

	for (j = 0; j < VNES_VIEWPORT_HEIGHT; j++) {
		for (i = 0; i < VNES_VIEWPORT_WIDTH; i++) {
			scr[(VNES_VIEWPORT_HEIGHT - j - 1) * VNES_VIEWPORT_WIDTH + i] = s[j * VNES_VIEWPORT_WIDTH + i];
		}
	}

	f=fopen(fname,"wb");
	fwrite(bmphead,sizeof(bmphead),1,f);
	fwrite(scr,VNES_VIEWPORT_WIDTH * VNES_VIEWPORT_HEIGHT*2,1,f);
	fclose(f);

	fprintf(stderr, "Screenshot saved!\n");
}

static void save_sdl_screenshot(void) {
	char fname[256];
	snprintf(fname, 256, "Screenshot_SDL_%ld.bmp", time(NULL));
	SDL_SaveBMP(surface, fname);
	fprintf(stderr, "Screenshot SDL saved!\n");
}
