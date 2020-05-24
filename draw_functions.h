<<<<<<< HEAD
#define _CRT_SECURE_NO_WARNINGS
=======
>>>>>>> devel
#include <SDL.h>
#include <SDL_ttf.h>
#include "../../../common/common_structures.h"

<<<<<<< HEAD
#define FONT_SIZE 16
#define POSITION_Y 35
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 640
=======
#define FONT_SIZE 10
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
>>>>>>> devel

//struktura za³adowanych zmiennych SDL
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
	//tekstura œciany
	SDL_Surface* wall;
	//tekstura pod³ogi (wolnej przestrzeni)
	SDL_Surface* floor;
	//tablica tekstur graczy
	SDL_Surface* player_surfs[NUMBER_OF_CLIENTS];
<<<<<<< HEAD
	//tablica kolorow graczy
	SDL_Color player_colors[NUMBER_OF_CLIENTS];
=======
>>>>>>> devel
	//tekstura skrzyni
	SDL_Surface* chest;
	//teksura umiejetnosci
	SDL_Surface* skill;
<<<<<<< HEAD
	//teksura zamra¿ania
	SDL_Surface* frozen;
	//ikona pocz¹tkowa
	SDL_Surface* icon;
	//wyjœcie z labiryntu
	SDL_Surface* exit;
=======
>>>>>>> devel
}SDL_package_type;

//nadaje pocz¹tkowe wartoœci paczce SDL
int initialize_package(SDL_package_type *package);

//rysuje tekst na pozycji (x, y)
void draw_text(const char* text, int position_x, int position_y, SDL_package_type package);

//dodaje wybran¹ powierzchniê do ekranu na pozycji (x, y)
void draw_surface(SDL_package_type* package, SDL_Surface* surface, int x, int y);

//rysuje mapê (labirynt, gracze, skarby)
void draw_map(SDL_package_type* package, Map_type* map);

//rysuje labirynt
void draw_labyrinth(SDL_package_type* package, Map_type* map);

//rysuje graczy
void draw_players(SDL_package_type* package, Map_type* map);

//rysuje skrzynie
<<<<<<< HEAD
void draw_chests(SDL_package_type* package, Map_type* map);

//rysuje umiejetnosci
void draw_skills(SDL_package_type* package, Map_type* map);

//rysuje punkty graczy
void draw_info(SDL_package_type* package, Map_type* map);

//rysuje ekran koncowy
void draw_game_over(SDL_package_type* package, Map_type* map);
=======
void draw_chests(SDL_package_type* package, Map_type* map);
>>>>>>> devel
