#include"connection.h"

void deserialize_map_fully(char* data, Map_type* map, int* everybody_ready, int* player_number, char* game_over)
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
        for (int j = 0; j < NUMBER_OF_TREASURES; j++)
        {
            memcpy(&(map->players[i].treasures[j]), data + position, sizeof(int));
            position += sizeof(int);
        }
        memcpy(&(map->players[i].skill), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].frozen), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].speed), data + position, sizeof(int));
        position += sizeof(int);
        memcpy(&(map->players[i].has_left), data + position, sizeof(int));
        position += sizeof(int);
    }
    for (int j = 0; j < map->size; j++)
    {
        for (int k = 0; k < map->size; k++)
        {
            memcpy(&(map->labyrinth[j][k]), data + position, sizeof(unsigned char));
            position += sizeof(unsigned char);
        }
    }
    memcpy(everybody_ready, data + position, sizeof(int));
    position += sizeof(int);
    memcpy(&(map->time), data + position, sizeof(int));
    position += sizeof(int);
    memcpy(player_number, data + position, sizeof(int));
    position += sizeof(int);
    memcpy(game_over, data + position, sizeof(char));
    position += sizeof(char);
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

void receive_full_data_from_server(SOCKET s, Map_type* map, int* everybody_ready, int* player_number, char* game_over)
{
    char* data = (char*)malloc(SIZE_OF_DATA);
    recv(s, data, SIZE_OF_DATA, 0);
    deserialize_map_fully(data, map, everybody_ready, player_number, game_over);
    free(data);
}

int ping_and_receive(SOCKET s, Map_type* map, int* everybody_ready, int* player_number, char* game_over)
{
    char buf[STRING_LENGTH];
    int latency = ping_server(s);
    recv(s, buf, STRING_LENGTH, 0);
    if (strcmp(buf, "full_map") == 0)
    {
        receive_full_data_from_server(s, map, everybody_ready, player_number, game_over);
    }
    return latency;
}

void receive_important_treasure_id_from_server(SOCKET s, int* important_treasure)
{
    char* int_holder = (char*)malloc(sizeof(int));
    recv(s, int_holder, sizeof(int), 0);
    memcpy(important_treasure, int_holder, sizeof(int));
    free(int_holder);
}

SOCKET connect_to_server(const char* address, char* nick, Map_type* map, int* important_treasure)
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
        recv(s, buf, STRING_LENGTH, 0);
        if (strcmp(buf, "OK") == 0)
        {
            strcpy(buf, nick);
            send(s, buf, STRING_LENGTH, 0);
            receive_labyrinth_from_server(s, map);
            receive_important_treasure_id_from_server(s, important_treasure);
            return s;
        }
        else
        {
            closesocket(s);
            WSACleanup();
            return 0;
        }
    }
}

void receive_labyrinth_from_server(SOCKET s, Map_type* map)
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