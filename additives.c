#include "additives.h"

void read_BMP(char* filename, Map_type* map)
{
	FILE* f = fopen(filename, "rb");

	if (f == NULL)
		return;

	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

	// extract image height and width from header
	map->size = *(int*)&info[18];

	int row_padded = (map->size * 3 + 3) & (~3);
	unsigned char** data = (char**)malloc(map->size * sizeof(unsigned char*));
	unsigned char* raw_pixel_data = (unsigned char*)malloc(row_padded * sizeof(unsigned char));

	for (int i = map->size - 1; i >= 0; i--)
	{
		data[i] = (char*)malloc(map->size * sizeof(unsigned char));
		fread(raw_pixel_data, sizeof(unsigned char), row_padded, f);
		for (int j = 0; j < map->size * 3; j += 3)
		{
			// Convert (B, G, R) to (R, G, B)
			if (((int)raw_pixel_data[j] + (int)raw_pixel_data[j + 1] + (int)raw_pixel_data[j + 2]) / 3 < 128)
			{
				data[i][j / 3] = 0;
			}
			else
			{
				data[i][j / 3] = 1;
			}
		}
	}
	free(raw_pixel_data);
	fclose(f);
	map->labyrinth = data;
}

void set_treasures(Map_type* map)
{
	int x_random, y_random;
	for (int i = 0; i < NUMBER_OF_TREASURES; i++)
	{
		do
		{
			x_random = rand() % map->size;
			y_random = rand() % map->size;

		} while (map->labyrinth[x_random][y_random] != FLOOR);
		map->labyrinth[x_random][y_random] = TREASURE_OFFSET + i;
	}
}

void set_exit(Map_type* map)
{
	int x_random, y_random;
	do
	{
		x_random = rand() % map->size;
		y_random = rand() % map->size;

	} while (map->labyrinth[x_random][y_random] != FLOOR);
	map->labyrinth[x_random][y_random] = EXIT;
}

int find_best_player_index_excluding_arg_player(Map_type* map, int player)
{
	int best_score = 0;
	int best_index = -1;
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		if (i == player)
		{
			continue;
		}
		if (map->players[i].connected && map->players[i].has_left == 0)
		{
			if (map->players[i].points > best_score)
			{
				best_index = i;
				best_score = map->players[i].points;
			}
		}
	}
	if (best_index == player)
	{
		best_index = (best_index + 1) % NUMBER_OF_CLIENTS;
	}
	return best_index;
}
