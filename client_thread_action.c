#include "client_thread_action.h"

void disconnect(Thread_args* arguments)
{
	int player_number = arguments->player_number;
	printf("Player number %d has left\n", player_number);
	if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		arguments->map->players[player_number].connected=0;
		arguments->map->players[player_number].ready=0;
	}
	ReleaseMutex(arguments->map_mutex);
}

void connect_to_client(Thread_args* arguments)
{
	char buf[STRING_LENGTH];
	int player_number = arguments->player_number;
	SOCKET client_socket = arguments->map->players[player_number].socket;
	strcpy(buf, "OK");
	send(client_socket, buf, STRING_LENGTH, 0);
	recv(client_socket, buf, STRING_LENGTH, 0);
	if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		int x, y;
		arguments->map->players[player_number].points = 0;
		do
		{
			x = rand() % arguments->map->size;
			y = rand() % arguments->map->size;
		} while (arguments->map->labyrinth[y][x] == WALL);
		arguments->map->players[player_number].x = x * TEXTURE_SIZE;
		arguments->map->players[player_number].y = y * TEXTURE_SIZE;
		arguments->map->players[player_number].connected = 1;
		arguments->map->players[player_number].ready = 0;
		strcpy(arguments->map->players[player_number].nick, buf);
	}
	ReleaseMutex(arguments->map_mutex);
	send_labyrinth_to_client(client_socket, arguments->map);
	send_important_treasure_id_to_client(client_socket, arguments->map->players[player_number].important_treasure);
}

void end_game(Thread_args* arguments)
{
	int player_number = arguments->player_number;
	int rd = 1;
	if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		arguments->map->players[player_number].has_left = 1;
	}
	ReleaseMutex(arguments->map_mutex);
	if (WaitForSingleObject(arguments->ready_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		rd = 1;
		for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
		{
			if (arguments->map->players[i].connected)
			{
				if (!arguments->map->players[i].has_left)
				{
					rd = 0;
				}
			}
		}
		*(arguments->everybody_left) = rd;
	}
	ReleaseMutex(arguments->ready_mutex);
}

void get_skill(Thread_args* arguments)
{
	int player_number = arguments->player_number;
	int left = arguments->map->players[player_number].x/TEXTURE_SIZE;
	int top = arguments->map->players[player_number].y/TEXTURE_SIZE;
	int right = (arguments->map->players[player_number].x+TEXTURE_SIZE-1)/TEXTURE_SIZE;
	int bottom = (arguments->map->players[player_number].y+TEXTURE_SIZE-1)/TEXTURE_SIZE;
	int skill_index;
	for(int i=SKILL_OFFSET;i<SKILL_OFFSET+NUMBER_OF_SKILLS;i++)
	{
		if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			if (left == right && top == bottom && arguments->map->labyrinth[top][left] == i)
			{
				skill_index = i - SKILL_OFFSET;
				arguments->map->players[player_number].skill = skill_index;
				arguments->map->labyrinth[top][left] = FLOOR;
				arguments->map->skills_number--;
			} 
		}
		ReleaseMutex(arguments->map_mutex);
	}
}
void get_ping_initialize(Thread_args* arguments)
{
	char buf[STRING_LENGTH];
	int player_number = arguments->player_number;
	SOCKET client_socket = arguments->map->players[player_number].socket;
	int rd = 1;
	strcpy(buf, "pong");
	send(client_socket, buf, STRING_LENGTH, 0);
	if (WaitForSingleObject(arguments->ready_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		rd = 1;
		for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
		{
			if (arguments->map->players[i].connected)
			{
				if (!arguments->map->players[i].ready)
				{
					rd = 0;
				}
			}
		}
		*(arguments->everybody_ready) = rd;
		if (arguments->map->time == -1 && rd == 1)
		{
			arguments->map->time = 120;
			ReleaseSemaphore(arguments->time_semaphore, 1, NULL);
		}
	}
	ReleaseMutex(arguments->ready_mutex);
	strcpy(buf, "full_map");
	send(client_socket, buf, STRING_LENGTH, 0);
	char* data = (char*)malloc(SIZE_OF_DATA);
	if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		serialize_map_fully(data, arguments->map, arguments->everybody_ready, &player_number, 0);
	}
	ReleaseMutex(arguments->map_mutex);
	send(client_socket, data, SIZE_OF_DATA, 0);
	free(data);
}

void get_chest(Thread_args* arguments)
{
	int player_number = arguments->player_number;
	int left = arguments->map->players[player_number].x/TEXTURE_SIZE;
	int top = arguments->map->players[player_number].y/TEXTURE_SIZE;
	int right = (arguments->map->players[player_number].x+TEXTURE_SIZE-1)/TEXTURE_SIZE;
	int bottom = (arguments->map->players[player_number].y+TEXTURE_SIZE-1)/TEXTURE_SIZE;
	int chest_index;
	for(int i=TREASURE_OFFSET;i<TREASURE_OFFSET+NUMBER_OF_TREASURES;i++)
	{
		if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			if (left == right && top == bottom && arguments->map->labyrinth[top][left] == i)
			{
				chest_index = i - TREASURE_OFFSET;
				arguments->map->players[player_number].treasures[chest_index] = 1;
				arguments->map->labyrinth[top][left] = FLOOR;
				if (chest_index == arguments->map->players[player_number].important_treasure)
				{
					arguments->map->players[player_number].points += 100;
				}
				else
				{
					arguments->map->players[player_number].points += 20;
				}
			} 
		}
		ReleaseMutex(arguments->map_mutex);
	}
}

int get_ping_game(Thread_args* arguments)
{
	char buf[STRING_LENGTH];
	int player_number = arguments->player_number;
	SOCKET client_socket = arguments->map->players[player_number].socket;
	if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		if (arguments->map->players[player_number].frozen > 0)
		{
			arguments->map->players[player_number].frozen--;
		}
		if (arguments->map->players[player_number].speed > 0)
		{
			arguments->map->players[player_number].speed--;
		}
	}
	ReleaseMutex(arguments->map_mutex);
	strcpy(buf, "pong");
	send(client_socket, buf, STRING_LENGTH, 0);
	strcpy(buf, "full_map");
	send(client_socket, buf, STRING_LENGTH, 0);
	char* data = (char*)malloc(SIZE_OF_DATA);
	if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		if (*(arguments->everybody_left) == 1 || arguments->map->time == 0)
		{
			serialize_map_fully(data, arguments->map, arguments->everybody_ready, &player_number, 1);
			ReleaseMutex(arguments->map_mutex);
			send(client_socket, data, SIZE_OF_DATA, 0);
			free(data);
			return 1;
		}
		else
		{
			serialize_map_fully(data, arguments->map, arguments->everybody_ready, &player_number, 0);
			ReleaseMutex(arguments->map_mutex);
			send(client_socket, data, SIZE_OF_DATA, 0);
			free(data);
		}
	}
	return 0;
}

void move_player(Thread_args* arguments, const char* buf)
{
	int player_number = arguments->player_number;
	int speed = SPEED;
	if (arguments->map->players[player_number].speed > 0)
	{
		speed = NEW_SPEED;
	}
	else
	{
		speed = SPEED;
	}
	if (strcmp(buf, "up") == 0)
	{
		printf("Player number %d sent UP\n", player_number);
		if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			if(arguments->map->players[player_number].y % NEW_SPEED != 0)
			{
				speed = SPEED;
			}
			arguments->map->players[player_number].y-= speed;
		}
		ReleaseMutex(arguments->map_mutex);
	}
	else if (strcmp(buf, "down") == 0)
	{
		printf("Player number %d sent DOWN\n", player_number);
		if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			if(arguments->map->players[player_number].y % NEW_SPEED != 0)
			{
				speed = SPEED;
			}
			arguments->map->players[player_number].y+= speed;
		}
		ReleaseMutex(arguments->map_mutex);
	}
	else if (strcmp(buf, "right") == 0)
	{
		printf("Player number %d sent RIGHT\n", player_number);
		if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			if(arguments->map->players[player_number].x % NEW_SPEED != 0)
			{
				speed = SPEED;
			}
			arguments->map->players[player_number].x+=speed;
		}
		ReleaseMutex(arguments->map_mutex);
	}
	else if (strcmp(buf, "left") == 0)
	{
		printf("Player number %d sent LEFT\n", player_number);
		if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			if(arguments->map->players[player_number].x % NEW_SPEED != 0)
			{
				speed = SPEED;
			}
			arguments->map->players[player_number].x-=speed;
		}
		ReleaseMutex(arguments->map_mutex);
	}
}

void use_skill(Thread_args* args)
{
	int player_number = args->player_number;
	Player_type* players = args->map->players;
	switch(args->map->players[player_number].skill)
	{
	case 0:
		if (WaitForSingleObject(args->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			players[player_number].skill=-1;
			int index = find_best_player_index_excluding_arg_player(args->map, player_number);
			if (index >= 0)
			{
				for (int i = 0; i < NUMBER_OF_TREASURES; i++)
				{
					if (players[index].treasures[i] == 1)
					{
						players[player_number].treasures[i] = 1;
						players[index].treasures[i] = 0;
						if (i == players[player_number].important_treasure)
						{
							players[player_number].points += 100;
						}
						else
						{
							players[player_number].points += 20;
						}
						if (i == players[index].important_treasure)
						{
							players[index].points -= 100;
						}
						else
						{
							players[index].points -= 20;
						}
						break;
					}
				}
			}
		}
		ReleaseMutex(args->map_mutex);
		break;
	case 1:
		if (WaitForSingleObject(args->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			players[player_number].skill=-1;
			int index = find_best_player_index_excluding_arg_player(args->map, player_number);
			if (index >= 0)
			{
				players[index].frozen = 250;
			}
		}
		ReleaseMutex(args->map_mutex);
		break;
	case 2:
		if (WaitForSingleObject(args->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			players[player_number].skill=-1;
			players[player_number].speed = 300;
		}
		ReleaseMutex(args->map_mutex);
		break;
	case 3:
		if (WaitForSingleObject(args->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			int x_temp, y_temp;
			players[player_number].skill=-1;
			int index = find_best_player_index_excluding_arg_player(args->map, player_number);
			if (index >= 0)
			{
				x_temp = players[index].x;
				y_temp = players[index].y;
				players[index].x = players[player_number].x;
				players[index].y = players[player_number].y;
				players[player_number].x = x_temp;
				players[player_number].y = y_temp;
			}
		}
		ReleaseMutex(args->map_mutex);
		break;
	}
}