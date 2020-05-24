#include"players.h"

void send_key_to_server(SOCKET s, const char* key)
{
    char buf[STRING_LENGTH];
    strcpy(buf, key);
    send(s, buf, STRING_LENGTH, 0);
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
