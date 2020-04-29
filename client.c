#define _CRT_SECURE_NO_WARNINGS
#define LOCALHOST "127.0.0.1"
#define PORT 420
#define SIZE_OF_DATA 1024
#define STRING_LENGTH 20

#include<winsock.h>
#include<stdio.h>
#include<time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#pragma comment(lib, "Ws2_32.lib")

//struktura danych gracza
typedef struct Player_type
{
    //sk�adowa pozioma pozycji gracza
    int x;
    //sk�adowa pionowa pozycji gracza
    int y;
    //punkty gracza
    int points;
    //nick gracza
    char nick[STRING_LENGTH];
}Player_type;

//struktura mapy
typedef struct Map_type
{
    //liczba graczy
    int number_of_players;
    //tablica graczy
    Player_type* players;
}Map_type;


void deserialize_map(char* data, Map_type* map)
{
    /*char int_holder[sizeof(int)];
    strcpy(data, "");
    for (int i = 0; i < map->number_of_players; i++)
    {
        memcpy(int_holder, &(map->players[i].x), sizeof(int));
        strcat(data, int_holder);
        memcpy(int_holder, &(map->players[i].y), sizeof(int));
        strcat(data, int_holder);
        memcpy(int_holder, &(map->players[i].points), sizeof(int));
        strcat(data, int_holder);
        strcat(data, map->players[i].nick);
    }*/
    int position = 0;
    for (int i = 0; i < map->number_of_players; i++)
    {
        memcpy(&(map->players[i].x), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].y), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].points), data + position, sizeof(int));
        position += sizeof(int);
    }
}


int ping_server(SOCKET s)
{
    int latency;
    char buf[STRING_LENGTH];
    clock_t start, end;
    strcpy(buf, "PING");
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

SOCKET connect_to_server(const char* address)
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
    if (result == SOCKET_ERROR)
    {
        return 0;
    }
    else
    {
        return s;
    }
}

void close_connection_to_server(SOCKET s)
{
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
    int byte_no = SIZE_OF_DATA;
    char* data = (char*)malloc(byte_no);
    recv(s, data, byte_no, 0);
    deserialize_map(data, map);
}

int main(int argc, char** argv)
{
    Map_type* map;
    TTF_Init();
    SDL_Window* win;
    SDL_Renderer *renderer;
    SDL_Texture* texture;
    SDL_Surface* surface;
    SDL_Color foregroundColor = { 255, 255, 255 };
    SDL_Color backgroundColor = { 0, 0, 255 };
    TTF_Font* font  = TTF_OpenFont("fonts/Lato-Regular.ttf", 12);
    SDL_Surface* text_surface;
    SDL_Event event;
    int is_running = 1;
    int latency;
   // SDL_Color color={0,0,0};

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL init error! Code: %s\n", SDL_GetError());
        return 1;
    }
    win = SDL_CreateWindow("420", 400, 400, 640, 480, SDL_WINDOW_SHOWN );
    if (win == NULL)
    {
        printf("Window creation error\n");
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(win);
        printf("Renderer creation error\n");
        SDL_Quit();
        return 1;
    }
    /*surface = SDL_LoadBMP("textures/ujc.bmp");
    if (surface == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        printf("Surface creation error\n");
        SDL_Quit();
        return 1;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        printf("Texture creation error\n");
        SDL_Quit();
        return 1;
    }*/
    SOCKET server = connect_to_server(LOCALHOST);
    int texW = 100;
    int texH = 20;
    SDL_Rect dstrect = { 0, 0, texW, texH };
    text_surface = TTF_RenderText_Solid(font, "This is my text.", foregroundColor);
    texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    while(is_running)
    {
        while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
			    if (event.key.keysym.sym == SDLK_UP) {
                    send_key_to_server(server, "up");
                    receive_data_from_server(server, map);
			    }
			    else if (event.key.keysym.sym == SDLK_DOWN) {
                    send_key_to_server(server, "down");
                    receive_data_from_server(server, map);
			    }
				else if (event.key.keysym.sym == SDLK_RIGHT) {	
                    send_key_to_server(server, "right");
                    receive_data_from_server(server, map);
				}
				else if (event.key.keysym.sym == SDLK_LEFT) {
                    send_key_to_server(server, "left");
                    receive_data_from_server(server, map);
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
                is_running = 0;
				break;
			};
		};
        latency = ping_server(server);
        receive_data_from_server(server, map);
        SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
    }
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

	return 0;
}