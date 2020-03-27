/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Compile with: g++ -g main.cpp -lSDL2 -lSDL2_image -lSDL2_ttf -o main
 *
 */

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

SDL_Window* wind = NULL;
SDL_Renderer* rend = NULL;
TTF_Font* font = NULL;
SDL_Texture* final = NULL;
FILE* inp = NULL;
int pixelsinsecond = 0;
int headeveryn = 5;
int padding = 15;
int windoww = 1280, windowh = 720;

void handle_abrt(int n) {
	printf("ABORT SDL last error: [%s]\n", SDL_GetError());
	return;
}

void renderback() {
	SDL_Texture* wt = SDL_GetRenderTarget(rend);
	SDL_Color wc;
	SDL_GetRenderDrawColor(rend, &wc.r, &wc.g, &wc.b, &wc.a);

	SDL_SetRenderDrawColor(rend, 0, 150, 0, 255);
	SDL_SetRenderTarget(rend, NULL);
	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, final, NULL, NULL);
	SDL_RenderPresent(rend);

	SDL_SetRenderDrawColor(rend, wc.r, wc.g, wc.b, wc.a);
	SDL_SetRenderTarget(rend, wt);
}
void save_texture(SDL_Renderer *ren, SDL_Texture *tex, const char *filename) {
    SDL_Texture *ren_tex;
    SDL_Surface *surf;
    int st, w, h, format;
    void *pixels;
    pixels  = NULL;
    surf    = NULL;
    ren_tex = NULL;
    format  = SDL_PIXELFORMAT_RGBA32;
    st = SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    if (st != 0) {
        SDL_Log("Failed querying texture: %s\n", SDL_GetError());
        goto cleanup;
    }
    ren_tex = SDL_CreateTexture(ren, format, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!ren_tex) {
        SDL_Log("Failed creating render texture: %s\n", SDL_GetError());
        goto cleanup;
    }
    st = SDL_SetRenderTarget(ren, ren_tex);
    if (st != 0) {
        SDL_Log("Failed setting render target: %s\n", SDL_GetError());
        goto cleanup;
    }
    SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(ren);
    st = SDL_RenderCopy(ren, tex, NULL, NULL);
    if (st != 0) {
        SDL_Log("Failed copying texture data: %s\n", SDL_GetError());
        goto cleanup;
    }
    pixels = malloc(w * h * SDL_BYTESPERPIXEL(format));
    if (!pixels) {
        SDL_Log("Failed allocating memory\n");
        goto cleanup;
    }
    st = SDL_RenderReadPixels(ren, NULL, format, pixels, w * SDL_BYTESPERPIXEL(format));
    if (st != 0) {
        SDL_Log("Failed reading pixel data: %s\n", SDL_GetError());
        goto cleanup;
    }
    surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, SDL_BITSPERPIXEL(format), w * SDL_BYTESPERPIXEL(format), format);
    if (!surf) {
        SDL_Log("Failed creating new surface: %s\n", SDL_GetError());
        goto cleanup;
    }
    st = SDL_SaveBMP(surf, filename);
    if (st != 0) {
        SDL_Log("Failed saving image: %s\n", SDL_GetError());
        goto cleanup;
    }
    SDL_Log("Saved texture as BMP to \"%s\"\n", filename);
cleanup:
    SDL_FreeSurface(surf);
    free(pixels);
    SDL_DestroyTexture(ren_tex);
}

int main(int argc, char** argv) {
	setbuf(stdout, 0);
	signal(SIGABRT, handle_abrt);
	if(argc < 1) {
		printf("Usage: %s <data file>\n", argv[0]);
		return 1;
	}

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_VideoInit(NULL);
	TTF_Init();
	int imgflags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
	IMG_Init(imgflags);

	font = TTF_OpenFont("font.ttf", 14);
	assert(font != NULL);

	inp = fopen(argv[1], "r");
	{
		int readed;
		size_t bufsize = 255;
		char* s = (char*)malloc(bufsize*sizeof(char));
		readed = getline(&s, &bufsize, inp);
		sscanf(s, "%d %d", &windoww, &windowh);
		free(s);
		printf("Size: %dx%d\n", windoww, windowh);
	}


	Uint32 WindowFlags = SDL_WINDOW_SHOWN;
	SDL_CreateWindowAndRenderer(windoww, windowh, WindowFlags, &wind, &rend);
	assert(wind != NULL);
	assert(rend != NULL);

	final = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, windoww, windowh);
	assert(final != NULL);
	SDL_SetRenderTarget(rend, final);
	SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
	SDL_RenderClear(rend);


	int afterlabel = 0;
	{
		char* labeltext = (char*)malloc(1024*sizeof(char));
		fgets(labeltext, 1024, inp);
		labeltext[strlen(labeltext)-1] = '\0';
		SDL_Surface* labels = TTF_RenderText_Blended(font, labeltext, {0, 0, 0, 255});
		free(labeltext);
		SDL_Texture* label = SDL_CreateTextureFromSurface(rend, labels);
		SDL_FreeSurface(labels);
		SDL_Rect lr;
		SDL_QueryTexture(label, NULL, NULL, &lr.w, &lr.h);
		lr.x = padding;
		lr.y = padding;
		afterlabel = padding + lr.h + padding + lr.h/2;
		SDL_RenderCopy(rend, label, NULL, &lr);
		SDL_DestroyTexture(label);
	}

	{
		int readed;
		size_t bufsize = 255;
		char* s = (char*)malloc(bufsize*sizeof(char));
		readed = getline(&s, &bufsize, inp);
		pixelsinsecond = atoi(s);
		free(s);
		printf("Pixels per second: %d\n", pixelsinsecond);
	}
	{
		int readed;
		size_t bufsize = 255;
		char* s = (char*)malloc(bufsize*sizeof(char));
		readed = getline(&s, &bufsize, inp);
		headeveryn = atoi(s);
		free(s);
		printf("Timestamp every %d steps\n", headeveryn);
	}
	{
		SDL_Rect tl_sec;
		tl_sec.x = padding;
		tl_sec.y = afterlabel;
		tl_sec.w = 1;
		tl_sec.h = 5;
		int contin = 0;
		for(int i=tl_sec.x; i<windoww; i+=pixelsinsecond) {
			if(contin == 0 || contin%headeveryn == 0) {
				int bufsize = 256;
				char* buf = (char*)malloc(bufsize*sizeof(char));
				snprintf(buf, bufsize, "%d", contin);
				SDL_Surface* timets = TTF_RenderText_Blended(font, buf, {0, 0, 0, 255});
				SDL_Texture* timet = SDL_CreateTextureFromSurface(rend, timets);
				SDL_FreeSurface(timets);
				SDL_Rect tr;
				SDL_QueryTexture(timet, NULL, NULL, &tr.w, &tr.h);
				tr.x = i;
				tr.y = afterlabel - tr.h;
				SDL_RenderCopy(rend, timet, NULL, &tr);
				SDL_DestroyTexture(timet);
			}
			tl_sec.x = i;
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
			SDL_RenderFillRect(rend, &tl_sec);
			contin++;
		}
	}

	{
		int readed;
		size_t bufsize = 512;
		char* buf = (char*)malloc(bufsize*sizeof(char));
		int count = 0;
		while ((readed = getline(&buf, &bufsize, inp)) != -1) {
			buf[strlen(buf)-1] = '\0';
			printf("Got line: (%zu) [%s]\n", readed, buf);
			int linenum, start, lenth;
			char* text;
			sscanf(buf, "%d %d %d %m[^\n]s", &linenum, &start, &lenth, &text);
			if(linenum >= 1)
				linenum--;
			SDL_Surface* blockts = TTF_RenderText_Blended(font, text, {0, 0, 0, 255});
			free(text);
			SDL_Texture* blockt = SDL_CreateTextureFromSurface(rend, blockts);
			SDL_FreeSurface(blockts);
			SDL_Rect tr = {0, 0, 0, 0};
			SDL_QueryTexture(blockt, NULL, NULL, &tr.w, &tr.h);
			SDL_Rect br = {start*pixelsinsecond + padding, afterlabel + 5 + padding, lenth*pixelsinsecond, tr.h};
			if(linenum != -1)
				br.y+=tr.h*linenum;
			else
				br.y+=tr.h*count;
			tr.x = br.x;
			tr.y = br.y;
			SDL_RenderCopy(rend, blockt, NULL, &tr);
			SDL_RenderDrawRect(rend, &br);
			SDL_DestroyTexture(blockt);
			count++;
		}
		free(buf);
	}


	renderback();
	save_texture(rend, final, "final.bmp");

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(wind);
	SDL_DestroyTexture(final);
	TTF_Quit();
	IMG_Quit();
	SDL_VideoQuit();
	SDL_Quit();
}
