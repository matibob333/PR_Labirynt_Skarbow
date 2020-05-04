#include "draw_functions.h"

void draw_text(const char* text, int position_x, int position_y, SDL_package_type package)
{
    SDL_Surface* text_surface;
	SDL_Rect dstrect = { position_x, position_y, strlen(text)*5, FONT_SIZE };
	text_surface = TTF_RenderText_Solid(package.font, text, package.foregroundColor);
	SDL_BlitSurface(text_surface, NULL, package.screen, &dstrect);
	SDL_FreeSurface(text_surface);
}

int initialize_package(SDL_package_type *package)
{
	package->foregroundColor = (SDL_Color){ 255, 255, 255 };
    package->backgroundColor = (SDL_Color){ 0, 0, 255 };
	package->font  = TTF_OpenFont("fonts/Lato-Regular.ttf", FONT_SIZE);
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL init error! Code: %s\n", SDL_GetError());
        return 1;
    }
    package->win = SDL_CreateWindow("420", 400, 400, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if (package->win == NULL)
    {
        printf("Window creation error\n");
        SDL_Quit();
        return 1;
    }
    package->renderer = SDL_CreateRenderer(package->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (package->renderer == NULL)
    {
        SDL_DestroyWindow(package->win);
        printf("Renderer creation error\n");
        SDL_Quit();
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(package->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(package->renderer, 0, 0, 0, 255);
    SDL_SetWindowTitle(package->win, "Labirynt skarbow");
    package->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (package->screen == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Surface creation error\n");
        SDL_Quit();
        return 1;
    }
    package->texture = SDL_CreateTexture(package->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (package->texture == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Texture creation error\n");
        SDL_Quit();
        return 1;
    }
    package->color = SDL_MapRGB(package->screen->format, 0x00, 0x00, 0x00);
    return 0;
}