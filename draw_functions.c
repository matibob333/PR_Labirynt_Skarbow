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
    package->player_colors[0] = (SDL_Color){ 255, 255, 0 };
    package->player_colors[1] = (SDL_Color){ 255, 0, 0 };
    package->player_colors[2] = (SDL_Color){ 0, 0, 255 };
    package->player_colors[3] = (SDL_Color){ 0, 255, 0 };
	package->font  = TTF_OpenFont("open-sans/OpenSans-Bold.ttf", FONT_SIZE);
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL init error! Code: %s\n", SDL_GetError());
        return 1;
    }
    package->win = SDL_CreateWindow("420", 400, 200, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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
    package->wall = SDL_LoadBMP("textures/wall.bmp");
    if (package->wall == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Wall creation error\n");
        SDL_Quit();
        return 1;
    }
    package->floor = SDL_LoadBMP("textures/free_space.bmp");
    if (package->floor == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Floor creation error\n");
        SDL_Quit();
        return 1;
    }
    package->chest = SDL_LoadBMP("textures/chest.bmp");
    if (package->chest == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Chest creation error\n");
        SDL_Quit();
        return 1;
    }
    package->skill = SDL_LoadBMP("textures/skill.bmp");
    if (package->skill == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Chest creation error\n");
        SDL_Quit();
        return 1;
    }
    package->frozen = SDL_LoadBMP("textures/frozen.bmp");
    if (package->frozen == NULL)
    {
        SDL_DestroyRenderer(package->renderer);
        SDL_DestroyWindow(package->win);
        printf("Chest creation error\n");
        SDL_Quit();
        return 1;
    }
    package->player_surfs[0] = SDL_LoadBMP("textures/player_yellow.bmp");
    package->player_surfs[1] = SDL_LoadBMP("textures/player_red.bmp");
    package->player_surfs[2] = SDL_LoadBMP("textures/player_blue.bmp");
    package->player_surfs[3] = SDL_LoadBMP("textures/player_green.bmp");
  
    for (int i = 0; i < NUMBER_OF_CLIENTS; i++) 
    {
        if (package->player_surfs[i] == NULL) 
        {
            SDL_DestroyRenderer(package->renderer);
            SDL_DestroyWindow(package->win);
            printf("Player creation error\n");
            SDL_Quit();
            return 1;
        }
    }

    return 0;
}

void draw_surface(SDL_package_type* package, SDL_Surface* surface, int x, int y)
{
	SDL_Rect dstrect = (SDL_Rect){ x, y, surface->w, surface->h };
	SDL_BlitSurface(surface, NULL, package->screen, &dstrect);
}

void draw_labyrinth(SDL_package_type* package, Map_type* map)
{
    for (int i = 0; i < map->size; i++)
    {
        for (int j = 0; j < map->size; j++)
        {
            if (map->labyrinth[i][j] == 0)
            {
                draw_surface(package, package->wall,TEXTURE_SIZE*j, TEXTURE_SIZE*i);
            }
            else
            {
                draw_surface(package, package->floor, TEXTURE_SIZE * j, TEXTURE_SIZE * i);
            }
        }
    }
}
void draw_players(SDL_package_type* package, Map_type* map)
{
    for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
    {
        if (map->players[i].connected == 1 && map->players[i].ready == 1) 
        {
            draw_surface(package, package->player_surfs[i], map->players[i].x, map->players[i].y );
            if (map->players[i].frozen > 0)
            {
                draw_surface(package, package->frozen, map->players[i].x, map->players[i].y);
            }
        }

    }
}
void draw_chests(SDL_package_type* package, Map_type* map)
{
    for (int i = 0; i < map->size; i++)
    {
        for (int j = 0; j < map->size; j++)
        {
            if (map->labyrinth[i][j] >= TREASURE_OFFSET && map->labyrinth[i][j] < TREASURE_OFFSET + NUMBER_OF_TREASURES)
            {
                draw_surface(package, package->chest,TEXTURE_SIZE*j, TEXTURE_SIZE*i);
            }
        }
    }
}

void draw_skills(SDL_package_type* package, Map_type* map)
{
    for (int i = 0; i < map->size; i++)
    {
        for (int j = 0; j < map->size; j++)
        {
            if (map->labyrinth[i][j] >= SKILL_OFFSET && map->labyrinth[i][j] < SKILL_OFFSET + NUMBER_OF_SKILLS)
            {
                draw_surface(package, package->skill, TEXTURE_SIZE * j, TEXTURE_SIZE * i);
            }
        }
    }

}
void draw_info(SDL_package_type* package, Map_type* map)
{
    int position_y = 20;
    char buf[128];
    char number_holder[10];
    sprintf(buf, "Czas: %d", map->time);
    draw_text(buf, map->size * TEXTURE_SIZE + 10, position_y, *package);
    position_y += 30;
    for(int i=0;i<NUMBER_OF_CLIENTS;i++)
    {
        if(map->players[i].connected)
        {
            SDL_Color temp = package->foregroundColor;
            package->foregroundColor = package->player_colors[i];
            draw_text(map->players[i].nick, map->size * TEXTURE_SIZE + 10, position_y, *package);
            position_y += 10;
            sprintf(buf, "Punkty: %d", map->players[i].points);
            draw_text(buf, map->size * TEXTURE_SIZE + 10, position_y, *package);
            position_y += 10;
            sprintf(buf, "Skarby: ");
            for(int j=0;j<NUMBER_OF_TREASURES;j++)
            {
                if(map->players[i].treasures[j]==1)
                {
                    sprintf(number_holder, "%d, ", j);
                    strcat(buf, number_holder);
				}
			}
            if (strlen(buf) > strlen("Skarby: "))
            {
                buf[strlen(buf) - 2] = '\0';
            }
            draw_text(buf, map->size * TEXTURE_SIZE + 10, position_y, *package);
            position_y += 10;
            sprintf(buf, "Umiejetnosc: %d", map->players[i].skill);
            draw_text(buf, map->size * TEXTURE_SIZE + 10, position_y, *package);
            position_y += 20;
            package->foregroundColor = temp;
        }
	}
}
void draw_map(SDL_package_type* package, Map_type* map)
{
    draw_labyrinth(package, map);
    draw_chests(package, map);
    draw_skills(package, map);
    draw_players(package, map);
    draw_info(package, map);
}