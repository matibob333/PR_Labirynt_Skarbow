#include <SDL.h>
#include <SDL_ttf.h>

#define FONT_SIZE 10
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct SDL_package_type
{
	//okno
	SDL_Window* win;
	//renderer
	SDL_Renderer* renderer;
	//font tekstu
	TTF_Font* font;
	//powierzchnia ekranu
	SDL_Surface* screen;
	//tekstura do wyrysowywania
	SDL_Texture * texture;
	//kolor pierwszego planu
	SDL_Color foregroundColor;
	// kolor t³a
	SDL_Color backgroundColor;
	//kolor typu int
	int color;
}SDL_package_type;

int initialize_package(SDL_package_type *package);

void draw_text(const char* text, int position_x, int position_y, SDL_package_type package);