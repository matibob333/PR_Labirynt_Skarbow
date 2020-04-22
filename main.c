#define _CRT_SECURE_NO_WARNINGS


#include<winsock.h>
#include<stdio.h>
#include<time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define LOCALHOST "127.0.0.1"
#define PORT 420

#pragma comment(lib, "Ws2_32.lib")

void ping_server(const char* address)
{
    SOCKET s;
    struct sockaddr_in sa;
    WSADATA wsas;
    WORD version;
    int result;
    char buf[30];
    version = MAKEWORD(2, 0);
    WSAStartup(version, &wsas);
    s = socket(AF_INET, SOCK_STREAM, 0);
    memset((void*)(&sa), 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = inet_addr(address);
    int latency;
    clock_t start, end;
    result = connect(s, (struct sockaddr FAR*) & sa, sizeof(sa));
    if (result == SOCKET_ERROR)
    {
        printf("Cannot connect to %s\n", address);
    }
    else
    {
        while (1)
        {
            strcpy(buf, "PING");
            send(s, buf, 30, 0);
            start = clock();
            if (recv(s, buf, 30, 0) > 0)
            {
                end = clock();
                latency = (end - start) / (CLOCKS_PER_SEC / 1000);
                buf[5] = '\0';
                printf("%s received after %d ms\n", buf, latency);
            }
            else
            {
                printf("Ping lost\n");
            }
            Sleep(2000);
        }
    }
    closesocket(s);
    WSACleanup();
}

void send_key_to_server(const char* address, const char* key)
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
    char buf[20];
    result = connect(s, (struct sockaddr FAR*) & sa, sizeof(sa));
    if (result == SOCKET_ERROR)
    {
        printf("Cannot connect to %s\n", address);
    }
    else
    {
        strcpy(buf, key);
        send(s, buf, 20, 0);
    }
    closesocket(s);
    WSACleanup();
}

int main(int argc, char** argv)
{
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
                    send_key_to_server(LOCALHOST, "up");
			    }
			    else if (event.key.keysym.sym == SDLK_DOWN) {
                    send_key_to_server(LOCALHOST, "down");
			    }
				else if (event.key.keysym.sym == SDLK_RIGHT) {	
                    send_key_to_server(LOCALHOST, "right");
				}
				else if (event.key.keysym.sym == SDLK_LEFT) {
                    send_key_to_server(LOCALHOST, "left");
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
                is_running = 0;
				break;
			};
		};
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