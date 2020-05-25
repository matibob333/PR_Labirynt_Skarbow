#include "connection_to_client.h"

void send_labyrinth_to_client(SOCKET client, Map_type* map)
{
	char* int_holder = (char*)malloc(sizeof(int));
	memcpy(int_holder, &(map->size), sizeof(int));
	send(client, int_holder, sizeof(int), 0);
	for (int i = 0; i < map->size; i++)
	{
		send(client, map->labyrinth[i], map->size, 0);
	}
	free(int_holder);
}

void serialize_map_fully(char* data, Map_type* map, int* everybody_ready, int* player_number, char game_over)
{
	strcpy(data, "");
	int position = 0;
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		memcpy(data + position, &(map->players[i].x), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].y), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].ready), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].connected), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].points), sizeof(int));
		position += sizeof(int);
		int length = strlen(map->players[i].nick);
		memcpy(data + position, &length, sizeof(int));
		position += sizeof(int);
		memcpy(data + position, map->players[i].nick, length);
		position += length;
		for (int j = 0; j < NUMBER_OF_TREASURES; j++)
		{
			memcpy(data + position, &(map->players[i].treasures[j]), sizeof(int));
			position += sizeof(int);
		}
		memcpy(data + position, &(map->players[i].skill), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].frozen), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].speed), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].has_left), sizeof(int));
		position += sizeof(int);
	}
	for (int j = 0; j < map->size; j++)
	{
		for (int k = 0; k < map->size; k++)
		{
			memcpy(data + position, &(map->labyrinth[j][k]), sizeof(unsigned char));
			position += sizeof(unsigned char);
		}
	}
	memcpy(data + position, everybody_ready, sizeof(int));
	position += sizeof(int);
	memcpy(data + position, &(map->time), sizeof(int));
	position += sizeof(int);
	memcpy(data + position, player_number, sizeof(int));
	position += sizeof(int);
	memcpy(data + position, &game_over, sizeof(char));
	position += sizeof(char);
}

void serialize_map_important(char* data, Map_type* map)
{
	strcpy(data, "");
	int position = 0;
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		memcpy(data + position, &(map->players[i].x), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].y), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].connected), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].ready), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].skill), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].frozen), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].speed), sizeof(int));
		position += sizeof(int);
	}
	memcpy(data + position, &(map->time), sizeof(int));
	position += sizeof(int);
}

void send_important_treasure_id_to_client(SOCKET s, int id)
{
	char* int_holder = (char*)malloc(sizeof(int));
	memcpy(int_holder, &id, sizeof(int));
	send(s, int_holder, sizeof(int), 0);
	free(int_holder);
}