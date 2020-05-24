#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "../../../common/common_structures.h"

void send_labyrinth_to_client(SOCKET client, Map_type* map);

void serialize_map_fully(char* data, Map_type* map, int* everybody_ready, int* player_number, char game_over);

void send_important_treasure_id_to_client(SOCKET s, int id);

//void send_data_to_client(SOCKET client, Map_type* map, int* everybody_ready, int* player_number, char game_over)