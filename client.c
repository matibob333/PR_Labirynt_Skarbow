#define _CRT_SECURE_NO_WARNINGS

#include<winsock.h>
#include<stdio.h>
#include<time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "../common/common_structures.h"
#include "draw_functions.h"

#pragma comment(lib, "Ws2_32.lib")

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


void deserialize_map(char* data, Map_type* map)
{
    int position = 0;
    for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
    {
        memcpy(&(map->players[i].x), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].y), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].points), data + position, sizeof(int));
        position += sizeof(int);
    }
}

void deserialize_map_fully(char* data, Map_type* map, int *everybody_ready)
{
    int position = 0;
    for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
    {
        memcpy(&(map->players[i].x), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].y), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].ready), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].connected), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].points), data + position, sizeof(int));
        position += sizeof(int);
        int nick_length;
        memcpy(&nick_length, data + position, sizeof(int));
        position += sizeof(int);
        memcpy(map->players[i].nick, data + position, nick_length);
        position += nick_length;
    }
    memcpy(everybody_ready, data + position, sizeof(int));
}


int ping_server(SOCKET s)
{
    int latency = 0;
    char buf[STRING_LENGTH];
    clock_t start, end;
    strcpy(buf, "ping");
    send(s, buf, STRING_LENGTH, 0);
    start = clock();
    if (recv(s, buf, STRING_LENGTH, 0) > 0)
    {
        end = clock();
        latency = (end - start) / (CLOCKS_PER_SEC / 1000);
        buf[5] = '\0';
    }
    else
    {
        printf("Ping lost\n");
    }
    return latency;
}

void close_connection_to_server(SOCKET s)
{
    char buf[STRING_LENGTH];
    strcpy(buf, "disconnect");
    send(s, buf, STRING_LENGTH, 0);
    closesocket(s);
    WSACleanup();
}

void send_key_to_server(SOCKET s, const char* key)
{
    char buf[STRING_LENGTH];
    strcpy(buf, key);
    send(s, buf, STRING_LENGTH, 0);
}

void receive_data_from_server(SOCKET s, Map_type* map)
{
    char* data = (char*)malloc(SIZE_OF_DATA);
    recv(s, data, SIZE_OF_DATA, 0);
    deserialize_map(data, map);
    free(data);
}

void receive_full_data_from_server(SOCKET s, Map_type* map, int* everybody_ready)
{
    char* data = (char*)malloc(SIZE_OF_DATA);
    recv(s, data, SIZE_OF_DATA, 0);
    deserialize_map_fully(data, map, everybody_ready);
    free(data);
}

int ping_and_receive(SOCKET s, Map_type* map, int *everybody_ready)
{
    char buf[STRING_LENGTH];
    int latency = ping_server(s);
    recv(s, buf, STRING_LENGTH, 0);
    if (strcmp(buf, "map") == 0)
    {
        receive_data_from_server(s, map);
    }
    else if (strcmp(buf, "full_map") == 0)
    {
        receive_full_data_from_server(s, map, everybody_ready);
    }
    return latency;
}

void load_nickname(SDL_package_type package, char* nick) 
{
    SDL_bool done = SDL_FALSE;
    SDL_Event event;
    char text[128];
    sprintf(text, "Podaj swoj nick: ");
    sprintf(nick, "\0");
    while (!done)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        draw_text(nick, 100, 100, package);
        draw_text(text, 100, 50, package);
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

            }
        }
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        //SDL_RenderClear(package.renderer);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
    }
}

SOCKET connect_to_server(const char* address, char* nick)
{
    SOCKET s;
    struct sockaddr_in sa;
    WSADATA wsas;
    WORD version;
    int result;
    version = MAKEWORD(2, 0);
    WSAStartup(version, &wsas);
    s = socket(AF_INET, SOCK_STREAM, 0);
    memset((void*)(&sa), 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = inet_addr(address);
    result = connect(s, (struct sockaddr FAR*) & sa, sizeof(sa));
    char buf[STRING_LENGTH];
    if (result == SOCKET_ERROR)
    {
        return 0;
    }
    else
    {
        strcpy(buf, "connect");
        send(s, buf, STRING_LENGTH, 0);
        strcpy(buf, nick);
        send(s, buf, STRING_LENGTH, 0);
        return s;
    }
}

int initialize_players(Buttons_type buttons, SOCKET server, Map_type* map, SDL_package_type package)
{
    char buf[STRING_LENGTH];
    int latency;
    SDL_Event event;
    int ready = 0;
    int everybody_ready = 0;
    int quit = 0;
    while(!everybody_ready && !quit)
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
                draw_text(map->players[i].nick, 100, 100 + 20 * i, package);
            }
        }
        package.foregroundColor = (SDL_Color){ 255, 255, 255 };
        while (SDL_PollEvent(&event))
        {
            switch (event.type) 
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == buttons.action)
                {
                    for (int i = 0; i < NUMBER_OF_CLIENTS;i++)
                    {
                        if(ready==0)
                        {
                            strcpy(buf, "ready");
                            send(server, buf, STRING_LENGTH, 0);
                            ready = 1;
						}
                        else
                        {
                            strcpy(buf, "not_ready");
                            send(server, buf, STRING_LENGTH, 0);
                            ready = 0;
						}
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
        latency = ping_and_receive(server, map, &everybody_ready);
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        //SDL_RenderClear(package.renderer);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
	}
    return quit;
}

void start_game(Buttons_type buttons, SOCKET server, Map_type* map, SDL_package_type package)
{
    SDL_Event event;
    int is_running = 1;
    int latency;
    while(is_running)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        char buf[STRING_LENGTH];
        for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
        {
            if (map->players[i].connected == 1)
            {
                sprintf(buf, "Gracz numer %d", i);
                draw_text(buf, 100 * i, 0, package);
                sprintf(buf, "x: %d", map->players[i].x);
                draw_text(buf, 100 * i, 20, package);
                sprintf(buf, "y: %d", map->players[i].y);
                draw_text(buf, 100 * i, 40, package);
            }
        }
        while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
			    if (event.key.keysym.sym == buttons.up) {
                    send_key_to_server(server, "up");
			    }
			    else if (event.key.keysym.sym == buttons.down) {
                    send_key_to_server(server, "down");
			    }
				else if (event.key.keysym.sym == buttons.right) {	
                    send_key_to_server(server, "right");
				}
				else if (event.key.keysym.sym == buttons.left) {
                    send_key_to_server(server, "left");
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
                is_running = 0;
				break;
			};
		};
        latency = ping_and_receive(server, map, NULL);
        //SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        //SDL_RenderClear(package.renderer);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
    }
}

Buttons_type set_buttons(SDL_package_type package)
{
    Buttons_type buttons = (Buttons_type){ 0, 0, 0, 0, 0 };
    SDL_Event event;
    int choice = 1;
    while (choice)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        draw_text("Wybierz sterowanie", 100, 100, package);
        draw_text("1 - strzalki", 100, 120, package);
        draw_text("2 - WSAD", 100, 140, package);
        draw_text("3 - IJKL", 100, 160, package);
        draw_text("4 - TFGH", 100, 180, package);
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
        //SDL_RenderClear(package.renderer);
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
    load_nickname(package, nick);
    Buttons_type buttons_set = set_buttons(package);
    if(buttons_set.up != SDLK_0)
    {
        server = connect_to_server(LOCALHOST, nick);
        if(initialize_players(buttons_set, server, map, package) == 0)
        {
            start_game(buttons_set, server, map, package);
        }
	}  
    close_connection_to_server(server);
    SDL_FreeSurface(package.screen);
    SDL_DestroyTexture(package.texture);
    SDL_DestroyRenderer(package.renderer);
    SDL_DestroyWindow(package.win);
    SDL_Quit();

	return 0;
}