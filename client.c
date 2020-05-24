#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "draw_functions.h"
#include "connection.h"
#include "players.h"

#pragma comment(lib, "Ws2_32.lib")

int load_nickname(SDL_package_type package, char* nick)
{
    SDL_bool done = SDL_FALSE;
    SDL_Event event;
    char text[128];
    char title[128];
    int position_x = 350;
    int position_y = 100;
    sprintf(title, "Labirynt skarbów");
    sprintf(text, "Podaj swoj nick: ");
    sprintf(nick, "\0");
    int quit = 0;
    while (!done && !quit)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        draw_surface(&package, package.icon, position_x, position_y);
        draw_text(title, position_x, position_y + 140, package);
        draw_text(text, position_x, position_y + 220, package);
        draw_text(nick, position_x, position_y + 260, package);
        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    done = SDL_TRUE;
                }
                break;
            case SDL_TEXTINPUT:
                strcat(nick, event.text.text); //wczytywanie nicku
                break;
            case SDL_QUIT:
                quit = 1;
                break;
            }
        }
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
    }
    return quit;
}

int initialize_players(Buttons_type buttons, SOCKET server, Map_type* map, SDL_package_type package, int important_treasure)
{
    int position_x = 150;
    int position_y = 120;
    int latency;
    SDL_Event event;
    int ready = 0;
    int everybody_ready = 0;
    int quit = 0;
    int player_number;
    char treasure_name[20];
    while (everybody_ready != 1 && !quit)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
        {
            package.foregroundColor = (SDL_Color){ 255, 0, 0 };
            if (map->players[i].connected == 1)
            {
                if (map->players[i].ready == 1)
                {
                    package.foregroundColor = (SDL_Color){ 0, 255, 0 };
                }
                draw_text(map->players[i].nick, position_x + 90 * i, position_y, package);
            }
        }
        sprintf(treasure_name, "Twoj skarb: %d", important_treasure);
        draw_text(treasure_name, position_x + 200, position_y + 200, package);
        package.foregroundColor = (SDL_Color){ 255, 255, 255 };
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == buttons.action)
                {

                    if (ready == 0)
                    {
                        send_key_to_server(server, "ready");
                        ready = 1;
                    }
                    else
                    {
                        send_key_to_server(server, "not_ready");
                        ready = 0;
                    }
                }
                break;
            case SDL_KEYUP:
                break;
            case SDL_QUIT:
                quit = 1;
                break;
            }
        }
        char dummy;
        latency = ping_and_receive(server, map, &everybody_ready, &player_number, &dummy);
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
    }
    return quit;
}

void draw_end_game(SDL_package_type* package, Map_type* map)
{
    int quit = 0;
    SDL_Event event;
    for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
    {
        if (map->players[i].connected)
        {
            if (!map->players[i].has_left)
            {
                map->players[i].points /= 2;
            }
        }
    }
    while (!quit)
    {
        draw_game_over(package, map);
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                quit = 1;
                break;
            case SDL_QUIT:
                quit = 1;
                break;
            }
        }
        SDL_UpdateTexture(package->texture, NULL, package->screen->pixels, package->screen->pitch);
        SDL_RenderCopy(package->renderer, package->texture, NULL, NULL);
        SDL_RenderPresent(package->renderer);
    }
}

void start_game(Buttons_type buttons, SOCKET server, Map_type* map, SDL_package_type package)
{
    SDL_Event event;
    int is_running = 1;
    int latency;
    int dummy;
    int player_number = -1;
    char game_over = 0;
    while(is_running && game_over != 1)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        draw_map(&package, map);
        while (SDL_PollEvent(&event)) 
        {
			switch (event.type) 
            {
			case SDL_KEYDOWN:

                if(player_number != -1 && map->players[player_number].has_left == 0)
                {
			        if (event.key.keysym.sym == buttons.up) 
                    {
                        send_key_to_server(server, "up");
                        //make_proper_move(server, map, player_number, "up");
			        }
			        else if (event.key.keysym.sym == buttons.down) 
                    {
                        send_key_to_server(server, "down");
                        //make_proper_move(server, map, player_number, "down");
			        }
				    else if (event.key.keysym.sym == buttons.right)                 
                    {	
                        send_key_to_server(server, "right");
                        //make_proper_move(server, map, player_number, "right");
				    }
				    else if (event.key.keysym.sym == buttons.left) 
                    {
                        send_key_to_server(server, "left");
                        //make_proper_move(server, map, player_number, "left");
				    }
                    else if(event.key.keysym.sym == buttons.action)
                    {
                        if (check_if_on_exit(map, player_number))
                        {
                            send_key_to_server(server, "end_game");
                        }
                        else
                        {
                            send_key_to_server(server, "skill");
                        }
				    }
                }
                break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
                is_running = 0;
				break;
			};
		};
        if (player_number >= 0 && player_number <= 4)
        {
            latency = ping_and_receive(server, map, &dummy, &dummy, &game_over);
        }
        else
        {
            latency = ping_and_receive(server, map, &dummy, &player_number, &game_over);
        }
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
    }
}

Buttons_type set_buttons(SDL_package_type package)
{
    int position_x = 350;
    int position_y = 100;
    Buttons_type buttons = (Buttons_type){ 0, 0, 0, 0, 0 };
    SDL_Event event;
    int choice = 1;
    while (choice)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        draw_text("Wybierz sterowanie: ", position_x, position_y, package);
        draw_text("1 - strzalki + prawy shift", position_x, position_y + POSITION_Y, package);
        draw_text("2 - WSAD + E", position_x, position_y + 2*POSITION_Y, package);
        draw_text("3 - IJKL + O", position_x, position_y + 3*POSITION_Y, package);
        draw_text("4 - TFGH + Y", position_x, position_y + 4*POSITION_Y, package);
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_1)
                {   //zestaw 1 - strza³ki
                    buttons.up = SDLK_UP;
                    buttons.down = SDLK_DOWN;
                    buttons.right = SDLK_RIGHT;
                    buttons.left = SDLK_LEFT;
                    buttons.action = SDLK_RSHIFT;
                    choice = 0;
                }
                else if (event.key.keysym.sym == SDLK_2)
                {   //zestaw 2 - WSAD
                    buttons.up = SDLK_w;
                    buttons.down = SDLK_s;
                    buttons.right = SDLK_d;
                    buttons.left = SDLK_a;
                    buttons.action = SDLK_e;
                    choice = 0;
                }
                else if (event.key.keysym.sym == SDLK_3)
                {	//zestaw 3 - IJKL
                    buttons.up = SDLK_i;
                    buttons.down = SDLK_k;
                    buttons.right = SDLK_l;
                    buttons.left = SDLK_j;
                    buttons.action = SDLK_o;
                    choice = 0;
                }
                else if (event.key.keysym.sym == SDLK_4)
                {   //zestaw 4 - TFGH
                    buttons.up = SDLK_t;
                    buttons.down = SDLK_f;
                    buttons.right = SDLK_g;
                    buttons.left = SDLK_h;
                    buttons.action = SDLK_y;
                    choice = 0;
                }
                break;
            case SDL_KEYUP:
                break;
            case SDL_QUIT:
                choice = 0;
                buttons.up = SDLK_0;
                break;
            }
        }
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
    }
    return buttons;
}

void initialize_map(Map_type *map)
{
    map->players = (Player_type*)malloc(sizeof(Player_type) * NUMBER_OF_CLIENTS);
}

void free_map(Map_type* map)
{
    free(map->players);
    free(map);
}

int main(int argc, char** argv)
{
    Map_type* map = (Map_type*)malloc(sizeof(Map_type));
    initialize_map(map);
    TTF_Init();
    SDL_package_type package;
    SOCKET server = 0;
    if (initialize_package(&package) != 0)
    {
        printf("NIE DZIALA");
        SDL_Quit();
        return 1;
    }
    char* nick = (char*)malloc(STRING_LENGTH); 
    int important_treasure = -1;
    int quit = load_nickname(package, nick);
    if(quit)
    {
        SDL_FreeSurface(package.screen);
        SDL_DestroyTexture(package.texture);
        SDL_DestroyRenderer(package.renderer);
        SDL_DestroyWindow(package.win);
        SDL_Quit();
	    return 0;
	}
    Buttons_type buttons_set = set_buttons(package);
    if(buttons_set.up != SDLK_0)
    {
        server = connect_to_server(LOCALHOST, nick, map, &important_treasure);
        if (server != 0)
        {
            if (initialize_players(buttons_set, server, map, package, important_treasure) == 0)
            {
                start_game(buttons_set, server, map, package);
                draw_end_game(&package, map);
            }
            close_connection_to_server(server);
        }
	}
    SDL_FreeSurface(package.screen);
    SDL_DestroyTexture(package.texture);
    SDL_DestroyRenderer(package.renderer);
    SDL_DestroyWindow(package.win);
    SDL_Quit();

	return 0;
}