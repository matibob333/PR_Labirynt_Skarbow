
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "draw_functions.h"

#pragma comment(lib, "Ws2_32.lib")

void recive_labyrinth_from_server(SOCKET s, Map_type* map)
{
    char* int_holder = (char*)malloc(sizeof(int));
    recv(s, int_holder, sizeof(int), 0);
    memcpy(&(map->size), int_holder, sizeof(int));
    map->labyrinth = (unsigned char**)malloc(map->size * sizeof(unsigned char*));
    for (int i = 0; i < map->size; i++)
    {
        map->labyrinth[i] = (unsigned char*)malloc(map->size * (sizeof(unsigned char)));
        recv(s, map->labyrinth[i], map->size, 0);
    }
    free(int_holder);
}

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
        memcpy(&(map->players[i].connected), data + position, sizeof(int));
        position += sizeof(int);
    }
}

void deserialize_map_fully(char* data, Map_type* map, int *everybody_ready, int* player_number)
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
        map->players[i].nick[nick_length] = '\0';
        position += nick_length;
        for(int j=0;j<NUMBER_OF_TREASURES;j++)
		{
			memcpy(&(map->players[i].treasures[j]), data+position, sizeof(int));
			position+=sizeof(int);
		}
    }
    for(int j=0;j<map->size;j++)
	{
		for(int k=0;k<map->size;k++)
		{
            memcpy(&(map->labyrinth[j][k]), data+position, sizeof(unsigned char));
			position+=sizeof(unsigned char);
		}
	}
    memcpy(everybody_ready, data + position, sizeof(int));
    position += sizeof(int);
    memcpy(player_number, data + position, sizeof(int));
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

void receive_full_data_from_server(SOCKET s, Map_type* map, int* everybody_ready, int* player_number)
{
    char* data = (char*)malloc(SIZE_OF_DATA);
    recv(s, data, SIZE_OF_DATA, 0);
    deserialize_map_fully(data, map, everybody_ready, player_number);
    free(data);
}

int ping_and_receive(SOCKET s, Map_type* map, int *everybody_ready, int* player_number)
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
        receive_full_data_from_server(s, map, everybody_ready, player_number);
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

void receive_important_treasure_id_from_server(SOCKET s, int *important_treasure)
{
    char* int_holder = (char*)malloc(sizeof(int));
    recv(s, int_holder, sizeof(int), 0);
    memcpy(important_treasure, int_holder, sizeof(int));
    free(int_holder);
}

SOCKET connect_to_server(const char* address, char* nick, Map_type* map, int *important_treasure)
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
        recive_labyrinth_from_server(s,map);
        receive_important_treasure_id_from_server(s, important_treasure);
        return s;
    }
}
int initialize_players(Buttons_type buttons, SOCKET server, Map_type* map, SDL_package_type package, int important_treasure)
{
    char buf[STRING_LENGTH];
    int latency;
    SDL_Event event;
    int ready = 0;
    int everybody_ready = 0;
    int quit = 0;
    int player_number;
    char treasure_name[20];
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
        sprintf(treasure_name, "twoj skarb: %d", important_treasure);
        draw_text(treasure_name, 100, 300, package);
        package.foregroundColor = (SDL_Color){ 255, 255, 255 };
        while (SDL_PollEvent(&event))
        {
            switch (event.type) 
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == buttons.action)
                {
                    if(ready==0)
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
        latency = ping_and_receive(server, map, &everybody_ready, &player_number);
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        //SDL_RenderClear(package.renderer);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
        Sleep(16);
	}
    return quit;
}

void make_proper_move(SOCKET server, Map_type* map, int player_number, const char* command)
{
    int vertical;
    int horizontal;
    int delta_vertical;
    int delta_horizontal;
    int border_horizontal;
    int border_vertical;
    int vertical_border;
    int horizontal_border;
    
    if(strcmp(command,"up")==0)
    {
        delta_vertical = -SPEED;
        delta_horizontal = 0;
        border_horizontal = TEXTURE_SIZE - 1;
        border_vertical = 0;
	}
    else if(strcmp(command, "down")==0)
    {
        delta_vertical = SPEED + TEXTURE_SIZE - 1;
        delta_horizontal = 0;
        border_horizontal = TEXTURE_SIZE - 1;
        border_vertical = 0;
	}
    else if(strcmp(command, "right")==0)
    {
        delta_vertical = 0;
        delta_horizontal = SPEED + TEXTURE_SIZE - 1;
        border_vertical = TEXTURE_SIZE - 1;
        border_horizontal = 0;
	}
    else if(strcmp(command, "left")==0)
    {
        delta_vertical = 0;
        delta_horizontal = -SPEED;
        border_vertical = TEXTURE_SIZE - 1;
        border_horizontal = 0;
	}
    else
    {
        return;
	}
    if(player_number>-1)
    {
        vertical = (map->players[player_number].y + delta_vertical)/TEXTURE_SIZE;
        horizontal = (map->players[player_number].x + delta_horizontal)/ TEXTURE_SIZE;
        vertical_border = ((map->players[player_number].y + delta_vertical + border_vertical) / TEXTURE_SIZE);
        horizontal_border = ((map->players[player_number].x + delta_horizontal + border_horizontal) / TEXTURE_SIZE);
        if (map->labyrinth[vertical][horizontal] != 0 && map->labyrinth[vertical_border][horizontal_border] != 0)
        {
            if ((map->labyrinth[vertical][horizontal] >= NUMBER_OF_TREASURES && map->labyrinth[vertical][horizontal] <= 2 * NUMBER_OF_TREASURES - 1) || (map->labyrinth[vertical_border][horizontal_border] >= NUMBER_OF_TREASURES && map->labyrinth[vertical_border][horizontal_border] <= 2 * NUMBER_OF_TREASURES - 1))
            {
                send_key_to_server(server, command);
                send_key_to_server(server, "chest");
            }
            else
            {
                send_key_to_server(server, command);
            }
		}

	}

}

void start_game(Buttons_type buttons, SOCKET server, Map_type* map, SDL_package_type package)
{
    SDL_Event event;
    int is_running = 1;
    int latency;
    int dummy;
    int player_number = -1;
    
    while(is_running)
    {
        SDL_FillRect(package.screen, NULL, package.color);
        draw_map(&package, map);
        while (SDL_PollEvent(&event)) 
        {
			switch (event.type) 
            {
			case SDL_KEYDOWN:
                
			    if (event.key.keysym.sym == buttons.up) 
                {
                    make_proper_move(server, map, player_number, "up");
			    }
			    else if (event.key.keysym.sym == buttons.down) 
                {
                    make_proper_move(server, map, player_number, "down");
			    }
				else if (event.key.keysym.sym == buttons.right)                 
                {	
                    make_proper_move(server, map, player_number, "right");
				}
				else if (event.key.keysym.sym == buttons.left) 
                {
                    make_proper_move(server, map, player_number, "left");
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
                is_running = 0;
				break;
			};
		};
        latency = ping_and_receive(server, map, &dummy, &player_number);
        //SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
        SDL_UpdateTexture(package.texture, NULL, package.screen->pixels, package.screen->pitch);
        //SDL_RenderClear(package.renderer);
        SDL_RenderCopy(package.renderer, package.texture, NULL, NULL);
        SDL_RenderPresent(package.renderer);
        Sleep(16);
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
    int important_treasure = -1;
    load_nickname(package, nick);
    Buttons_type buttons_set = set_buttons(package);
    if(buttons_set.up != SDLK_0)
    {
        server = connect_to_server(LOCALHOST, nick, map, &important_treasure);
        if(initialize_players(buttons_set, server, map, package, important_treasure) == 0)
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