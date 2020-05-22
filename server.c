#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "../../../common/common_structures.h"

#pragma comment(lib, "Ws2_32.lib")

//struktura danych przesy³anych do klienta
typedef struct Thread_args
{
	//numer danego gracza
	int player_number;
	//wskaŸnik na wspóln¹ mapê
	Map_type* map;
	//wskaznik na zmienna, czy gracze sa gotowi
	int* everybody_ready;
	//uchwyt mutexa mapy
	HANDLE map_mutex;
	//uchwyt mutexa gotowoœci graczy
	HANDLE ready_mutex;
	//uchwyt semafora czasu
	HANDLE time_semaphore;
}Thread_args;

void send_labyrinth_to_client(SOCKET client, Map_type* map)
{
	char* int_holder = (char*)malloc(sizeof(int));
	memcpy(int_holder, &(map->size), sizeof(int));
	send(client, int_holder, sizeof(int), 0);
	for(int i=0;i<map->size;i++)
	{
		send(client, map->labyrinth[i], map->size, 0);
	}
	free(int_holder);
}

void read_BMP(char* filename, Map_type* map)
{
    FILE* f = fopen(filename, "rb");

    if(f == NULL)
        return;

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    map->size = *(int*)&info[18];

    int row_padded = (map->size*3 + 3) & (~3);
	unsigned char** data = (char**)malloc(map->size * sizeof(unsigned char*));
	unsigned char* raw_pixel_data = (unsigned char*)malloc(row_padded * sizeof(unsigned char));

    for(int i = map->size-1; i >= 0; i--)
    {
		data[i] = (char*)malloc(map->size * sizeof(unsigned char));
        fread(raw_pixel_data, sizeof(unsigned char), row_padded, f);
        for(int j = 0; j < map->size*3; j += 3)
        {
            // Convert (B, G, R) to (R, G, B)
            if(((int)raw_pixel_data[j] + (int)raw_pixel_data[j+1] + (int)raw_pixel_data[j+2])/3 < 128)
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
	for(int i=0;i<NUMBER_OF_TREASURES;i++)
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

//serializacja danych z mapy do tablicy bajtów w celu przes³ania
void serialize_map(char* data, Map_type* map)
{
	strcpy(data, "");
	int position = 0;
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		memcpy(data + position, &(map->players[i].x), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].y), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].points), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].connected), sizeof(int));
		position += sizeof(int);
	}
}

void serialize_map_fully(char* data, Map_type* map, int *everybody_ready, int *player_number)
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
		for(int j=0;j<NUMBER_OF_TREASURES;j++)
		{
			memcpy(data+position, &(map->players[i].treasures[j]), sizeof(int));
			position+=sizeof(int);
		}
		memcpy(data + position, &(map->players[i].skill), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].frozen), sizeof(int));
		position += sizeof(int);
		memcpy(data + position, &(map->players[i].speed), sizeof(int));
		position += sizeof(int);
	}
	for(int j=0;j<map->size;j++)
	{
		for(int k=0;k<map->size;k++)
		{
			memcpy(data+position, &(map->labyrinth[j][k]), sizeof(unsigned char));
			position+=sizeof(unsigned char);
		}
	}
	memcpy(data + position, everybody_ready, sizeof(int));
	position += sizeof(int);
	memcpy(data + position, &(map->time), sizeof(int));
	position += sizeof(int);
	memcpy(data + position, player_number, sizeof(int));
	position += sizeof(int);
}

void send_important_treasure_id_to_client(SOCKET s, int id)
{
	char* int_holder = (char*)malloc(sizeof(int));
	memcpy(int_holder, &id, sizeof(int));
	send(s, int_holder, sizeof(int), 0);
	free(int_holder); 
}

DWORD WINAPI time_thread(void* args)
{
	Thread_args* arguments = (Thread_args*)args;
	if (WaitForSingleObject(arguments->time_semaphore, INFINITE) == WAIT_OBJECT_0)
	{
		do
		{
			Sleep(1000);
			arguments->map->time--;
		} while (arguments->map->time > 0);
	}
	ReleaseSemaphore(arguments->time_semaphore, 1, NULL);
	free(arguments);
	return 0;
}

DWORD WINAPI skills_thread(void* args)
{
	Thread_args* arguments = (Thread_args*)args;
	int max_skills_number = 7;
	printf("Uruchomiono watek umiejetnosci\n");
	while (!(*(arguments->everybody_ready)))
	{
		Sleep(1000);
	}
	printf("Watek umiejetnosci zaczyna\n");
	while (*(arguments->everybody_ready))
	{
		Sleep(4000);
		int x, y, skill_id;
		do
		{
			x = rand() % arguments->map->size;
			y = rand() % arguments->map->size;
		} while (arguments->map->labyrinth[y][x] != FLOOR );
		skill_id = rand() % NUMBER_OF_SKILLS;
		if(WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			printf("Dodano umiejetnosc\n");
			if (arguments->map->skills_number<= max_skills_number)
			{
				arguments->map->labyrinth[y][x] = SKILL_OFFSET + skill_id;
				arguments->map->skills_number++;
				printf("Dodano umiejetnosc\n");
			}
		}
		ReleaseMutex(arguments->map_mutex);
	}
	free(arguments);
	return 0;
}

int find_best_player_index_excluding_arg_player(Map_type* map, int player)
{
	int best_score = 0;
	int best_index = 0;
	for(int i=0;i<NUMBER_OF_CLIENTS;i++)
	{
		if(i==player)
		{
			continue;
		}
		if(map->players[i].connected)
		{
			if(map->players[i].points > best_score)
			{
				best_index = i;
				best_score = map->players[i].points;
			}
		}
	}
	if(best_index == player)
	{
		best_index = (best_index + 1) % NUMBER_OF_CLIENTS;
	}
	return best_index;
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
			for(int i=0;i<NUMBER_OF_TREASURES;i++)
			{
				if(players[index].treasures[i]==1)
				{
					players[player_number].treasures[i] = 1;
					players[index].treasures[i] = 0;
					if(i==players[player_number].important_treasure)
					{
						players[player_number].points += 100;
					}
					else
					{
						players[player_number].points += 20;
					}
					if(i==players[index].important_treasure)
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
		ReleaseMutex(args->map_mutex);
		break;
	case 1:
		if (WaitForSingleObject(args->map_mutex, INFINITE) == WAIT_OBJECT_0)
		{
			players[player_number].skill=-1;
			int index = find_best_player_index_excluding_arg_player(args->map, player_number);
			players[index].frozen = 250;
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
			x_temp = players[index].x;
			y_temp = players[index].y;
			players[index].x = players[player_number].x;
			players[index].y = players[player_number].y;
			players[player_number].x = x_temp;
			players[player_number].y = y_temp;
		}
		ReleaseMutex(args->map_mutex);
		break;
	}
}

DWORD WINAPI client_thread(void* args)
{
	char buf[STRING_LENGTH];
	int lost_client = 0;
	Thread_args* arguments = (Thread_args*)args;
	int player_number = arguments->player_number;
	SOCKET client_socket = arguments->map->players[player_number].socket;
	int byte_no = SIZE_OF_DATA;
	int rd;
	srand(time(NULL) * GetCurrentThreadId());
	if(*(arguments->everybody_ready))
	{
		recv(client_socket, buf, STRING_LENGTH, 0);
		if(strcmp(buf, "connect")==0)
		{
			strcpy(buf, "game_started");
			send(client_socket, buf, STRING_LENGTH, 0);
			lost_client = 1;
		}
	}
	while (!(*(arguments->everybody_ready)))
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
			//gracz do³¹cza do gry
			if (strcmp(buf, "connect")==0)
			{
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
					arguments->map->players[player_number].x = x*TEXTURE_SIZE;
					arguments->map->players[player_number].y = y*TEXTURE_SIZE;
					arguments->map->players[player_number].connected = 1;
					arguments->map->players[player_number].ready = 0;
					strcpy(arguments->map->players[player_number].nick, buf);
				}
				ReleaseMutex(arguments->map_mutex);
				send_labyrinth_to_client(client_socket, arguments->map);
				send_important_treasure_id_to_client(client_socket, arguments->map->players[player_number].important_treasure);
			}
			//gracz zg³asza gotowoœæ
			else if (strcmp(buf, "ready")==0)
			{
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].ready = 1;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			//gracz zg³asza brak gotowoœci
			else if (strcmp(buf, "not_ready")==0)
			{
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].ready = 0;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			//gracz od³¹cza siê od rozrywki
			else if (strcmp(buf, "disconnect")==0)
			{
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].connected = 0;
				}
				ReleaseMutex(arguments->map_mutex);
				//zakoñczenie w¹tku
				free(arguments);
				return 0;
			}
			else if (strcmp(buf, "ping")==0)
			{
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
					serialize_map_fully(data, arguments->map, arguments->everybody_ready, &player_number);
				}
				ReleaseMutex(arguments->map_mutex);
				send(client_socket, data, SIZE_OF_DATA, 0);
				free(data);
			}
			else
			{
				//TODO
			}
		}
	}
	int speed = SPEED;
	while (!lost_client)
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
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
				printf("Player number %d sent GURA\n", player_number);
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
				printf("Player number %d sent DUU\n", player_number);
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
				printf("Player number %d sent PRAWO\n", player_number);
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
				printf("Player number %d sent LEWO\n", player_number);
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
			else if (strcmp(buf, "ping") == 0)
			{
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
					serialize_map_fully(data, arguments->map, arguments->everybody_ready, &player_number);
				}
				ReleaseMutex(arguments->map_mutex);
				send(client_socket, data, SIZE_OF_DATA, 0);
				free(data);
			}
			else if (strcmp(buf, "chest")==0)
			{
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
			else if (strcmp(buf, "get_skill")==0)
			{
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
			else if (strcmp(buf, "end_game") == 0)
			{
				
			}
			else if(strcmp(buf, "skill")==0)
			{
				use_skill(arguments);
			}
			else if (strcmp(buf, "disconnect")==0)
			{
				printf("Player number %d has left\n", player_number);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].connected=0;
					arguments->map->players[player_number].ready=0;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			else
			{
				printf("NIEZROZUMIALE\n");
			}
		}
		else
		{
			printf("Lost connection to client\n");
			lost_client = 1;
		}
	}
	free(arguments);
	return 0;
}

int main()
{
	srand(time(NULL) * GetCurrentThreadId());	
	HANDLE map_mutex, ready_mutex, time_semaphore;
	map_mutex = CreateMutex(NULL, FALSE, NULL);
	ready_mutex = CreateMutex(NULL, FALSE, NULL);
	time_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	Map_type map;
	read_BMP("labyrinth.bmp", &map);
	set_exit(&map);
	set_treasures(&map);
	map.skills_number = 0;
	map.time = -1;
	int everybody_ready = 0;
	Thread_args *thread_args;
	Player_type players[NUMBER_OF_CLIENTS];
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++) 
	{
		players[i].connected = 0;
		strcpy(players[i].nick, " ");
		players[i].ready = 0;
		players[i].x = 0;
		players[i].y = 0;
		players[i].points = 0;
		players[i].skill = -1;
		int continue_drawing = 0;
		do
		{
			continue_drawing = 0;
			players[i].important_treasure = rand() % NUMBER_OF_TREASURES;
			for(int j=0; j < i; j++)
			{
				if (players[i].important_treasure == players[j].important_treasure)
				{
					continue_drawing = 1;
				}
			}
		} while (continue_drawing);
		for (int j = 0; j < NUMBER_OF_TREASURES; j++)
		{
			players[i].treasures[j] = 0;
		}

	}
	map.players = players;
	WSADATA wsas;
	int result;
	WORD wersja;
	wersja = MAKEWORD(1, 1);
	result = WSAStartup(wersja, &wsas);
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sa;
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	result = bind(s, (struct sockaddr FAR*) & sa, sizeof(sa));
	if (result != 0)
	{
		printf("Cannot bind socket");
		exit(0);
	}
	result = listen(s, NUMBER_OF_CLIENTS);

	SOCKET si;
	struct sockaddr_in sc;
	int lenc;
	char buf[STRING_LENGTH] = "PONG";
	DWORD id;
	thread_args = (Thread_args*)malloc(sizeof(Thread_args));
	thread_args->everybody_ready = &everybody_ready;
	thread_args->map_mutex = map_mutex;
	thread_args->ready_mutex = ready_mutex;
	thread_args->time_semaphore = time_semaphore;
	thread_args->map = &map;
	CreateThread(NULL, 0, skills_thread, (void*)thread_args, 0, &id);
	thread_args = (Thread_args*)malloc(sizeof(Thread_args));
	thread_args->everybody_ready = &everybody_ready;
	thread_args->map_mutex = map_mutex;
	thread_args->ready_mutex = ready_mutex;
	thread_args->time_semaphore = time_semaphore;
	thread_args->map = &map;
	CreateThread(NULL, 0, time_thread, (void*)thread_args, 0, &id);
	while (1)
	{
		lenc = sizeof(sc);
		si = accept(s, (struct sockaddr FAR*) &sc, &lenc);
		printf("Client accepted\n");
		
		thread_args = (Thread_args*)malloc(sizeof(Thread_args));
		thread_args->everybody_ready = &everybody_ready;
		thread_args->map_mutex = map_mutex;
		thread_args->ready_mutex = ready_mutex;
		thread_args->time_semaphore = time_semaphore;
		for(int i=0;i<NUMBER_OF_CLIENTS;i++)
		{
			if(!players[i].connected)
			{
				players[i].socket=si;
				thread_args->player_number = i;
				break;
			}
		}
		thread_args->map = &map;
		CreateThread(NULL, 0, client_thread, (void*)thread_args,0, &id);
	}
	CloseHandle(time_semaphore);
	CloseHandle(map_mutex);
	CloseHandle(ready_mutex);
	free(map.labyrinth);
	closesocket(si);
	WSACleanup();
	return 0;
}