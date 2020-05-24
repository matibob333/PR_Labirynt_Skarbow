#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <SDL_ttf.h>
#include "../../../common/common_structures.h"

typedef struct Buttons_type
{
    //przycisk ruchu w górê
    SDL_Keycode up;
    //przycisk ruchu w dó³
    SDL_Keycode down;
    //przycisk ruchu w prawo
    SDL_Keycode right;
    //przycisk ruchu w lewo
    SDL_Keycode left;
    //przycisk akcji
    SDL_Keycode action;
}Buttons_type;

void send_key_to_server(SOCKET s, const char* key);

int check_if_on_exit(Map_type* map, int player_number);

