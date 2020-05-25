#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <SDL_ttf.h>
#include "../../../common/common_structures.h"
#include<time.h>

int ping_server(SOCKET s);

void close_connection_to_server(SOCKET s);

void receive_full_data_from_server(SOCKET s, Map_type* map, int* everybody_ready, int* player_number, char* game_over);

void receive_important_data_from_server(SOCKET s, Map_type* map);

int ping_and_receive(SOCKET s, Map_type* map, int* everybody_ready, int* player_number, char* game_over);

void receive_important_treasure_id_from_server(SOCKET s, int* important_treasure);

SOCKET connect_to_server(const char* address, char* nick, Map_type* map, int* important_treasure);

void receive_labyrinth_from_server(SOCKET s, Map_type* map);

void deserialize_map_fully(char* data, Map_type* map, int* everybody_ready, int* player_number, char* game_over);

void deserialize_map_important(char* data, Map_type* map);