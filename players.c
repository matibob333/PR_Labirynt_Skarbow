#include"players.h"

void send_key_to_server(SOCKET s, const char* key)
{
    char buf[STRING_LENGTH];
    strcpy(buf, key);
    send(s, buf, STRING_LENGTH, 0);
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
    int speed = SPEED;
    if (map->players[player_number].speed > 0)
    {
        speed = NEW_SPEED;
    }

    if (strcmp(command, "up") == 0)
    {
        if (map->players[player_number].y % NEW_SPEED != 0)
        {
            speed = SPEED;
        }
        delta_vertical = -speed;
        delta_horizontal = 0;
        border_horizontal = TEXTURE_SIZE - 1;
        border_vertical = 0;
    }
    else if (strcmp(command, "down") == 0)
    {
        if (map->players[player_number].y % NEW_SPEED != 0)
        {
            speed = SPEED;
        }
        delta_vertical = speed + TEXTURE_SIZE - 1;
        delta_horizontal = 0;
        border_horizontal = TEXTURE_SIZE - 1;
        border_vertical = 0;
    }
    else if (strcmp(command, "right") == 0)
    {
        if (map->players[player_number].x % NEW_SPEED != 0)
        {
            speed = SPEED;
        }
        delta_vertical = 0;
        delta_horizontal = speed + TEXTURE_SIZE - 1;
        border_vertical = TEXTURE_SIZE - 1;
        border_horizontal = 0;
    }
    else if (strcmp(command, "left") == 0)
    {
        if (map->players[player_number].x % NEW_SPEED != 0)
        {
            speed = SPEED;
        }
        delta_vertical = 0;
        delta_horizontal = -speed;
        border_vertical = TEXTURE_SIZE - 1;
        border_horizontal = 0;
    }
    else
    {
        return;
    }
    if (player_number > -1)
    {
        vertical = (map->players[player_number].y + delta_vertical) / TEXTURE_SIZE;
        horizontal = (map->players[player_number].x + delta_horizontal) / TEXTURE_SIZE;
        vertical_border = ((map->players[player_number].y + delta_vertical + border_vertical) / TEXTURE_SIZE);
        horizontal_border = ((map->players[player_number].x + delta_horizontal + border_horizontal) / TEXTURE_SIZE);
        if (map->labyrinth[vertical][horizontal] != WALL && map->labyrinth[vertical_border][horizontal_border] != WALL)
        {
            if ((map->labyrinth[vertical][horizontal] >= TREASURE_OFFSET && map->labyrinth[vertical][horizontal] < TREASURE_OFFSET + NUMBER_OF_TREASURES) || (map->labyrinth[vertical_border][horizontal_border] >= TREASURE_OFFSET && map->labyrinth[vertical_border][horizontal_border] < TREASURE_OFFSET + NUMBER_OF_TREASURES))
            {
                send_key_to_server(server, command);
                send_key_to_server(server, "chest");
            }
            else if ((map->labyrinth[vertical][horizontal] >= SKILL_OFFSET && map->labyrinth[vertical][horizontal] < SKILL_OFFSET + NUMBER_OF_SKILLS) || (map->labyrinth[vertical_border][horizontal_border] >= SKILL_OFFSET && map->labyrinth[vertical_border][horizontal_border] < SKILL_OFFSET + NUMBER_OF_SKILLS))
            {
                send_key_to_server(server, command);
                send_key_to_server(server, "get_skill");
            }
            else
            {
                send_key_to_server(server, command);
            }
        }
    }
}

int check_if_on_exit(Map_type* map, int player_number)
{
    int left = map->players[player_number].x / TEXTURE_SIZE;
    int top = map->players[player_number].y / TEXTURE_SIZE;
    int right = (map->players[player_number].x + TEXTURE_SIZE - 1) / TEXTURE_SIZE;
    int bottom = (map->players[player_number].y + TEXTURE_SIZE - 1) / TEXTURE_SIZE;
    if (left == right && top == bottom && map->labyrinth[top][left] == EXIT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
