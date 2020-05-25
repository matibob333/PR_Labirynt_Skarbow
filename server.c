#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "../../../common/common_structures.h"
#include"connection_to_client.h"
#include"additives.h"
#include"client_thread_action.h"

#pragma comment(lib, "Ws2_32.lib")

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
	while (arguments->map->time != 0 && *(arguments->everybody_left) == 0)
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
				if(WaitForSingleObject(arguments->flags_mutex, INFINITE)==WAIT_OBJECT_0)
				{
					for(int i=0;i<NUMBER_OF_CLIENTS;i++)
					{
						arguments->flags[i] = 2;
					}
				}
				ReleaseMutex(arguments->flags_mutex);
			}
		}
		ReleaseMutex(arguments->map_mutex);
	}
	free(arguments);
	return 0;
}

DWORD WINAPI client_thread(void* args)
{
	char buf[STRING_LENGTH];
	int lost_client = 0;
	Thread_args* arguments = (Thread_args*)args;
	int player_number = arguments->player_number;
	SOCKET client_socket = arguments->map->players[player_number].socket;
	int byte_no = SIZE_OF_DATA;
	srand((unsigned int)time(NULL) * GetCurrentThreadId());
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
				connect_to_client(arguments);
				if (WaitForSingleObject(arguments->flags_mutex) == WAIT_OBJECT_0)
				{
					for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
					{
						arguments->flags[i] = 2;
					}
				}
				ReleaseMutex(arguments->flags_mutex);
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
				disconnect(arguments);
				if (WaitForSingleObject(arguments->flags_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
					{
						arguments->flags[i] = 2;
					}
				}
				ReleaseMutex(arguments->flags_mutex);
				//zakoñczenie w¹tku
				free(arguments);
				return 0;
			}
			else if (strcmp(buf, "ping")==0)
			{
				get_ping_initialize(arguments);
			}
		}
	
	}
	int speed = SPEED; 
	while (!lost_client)
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
			if(strcmp(buf, "up")==0 || strcmp(buf, "down")==0 || strcmp(buf, "right")==0 || strcmp(buf, "left")==0)
			{
				move_player(arguments, buf);
			}
			else if (strcmp(buf, "ping") == 0)
			{
				if (get_ping_game(arguments) == 1)
				{
					break;
				}
			}
			/*else if (strcmp(buf, "chest")==0)
			{
				get_chest(arguments);
			}
			else if (strcmp(buf, "get_skill")==0)
			{
				get_skill(arguments);
			}*/
			else if (strcmp(buf, "end_game") == 0)
			{
				end_game(arguments);
				if (WaitForSingleObject(arguments->flags_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
					{
						arguments->flags[i] = 2;
					}
				}
				ReleaseMutex(arguments->flags_mutex);
			}
			else if(strcmp(buf, "skill")==0)
			{
				use_skill(arguments);
			}
			else if (strcmp(buf, "disconnect")==0)
			{
				disconnect(arguments);
				if (WaitForSingleObject(arguments->flags_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
					{
						arguments->flags[i] = 2;
					}
				}
				ReleaseMutex(arguments->flags_mutex);
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
			//arguments->map->players[player_number].connected = 0;
		}
	}
	
	free(arguments);
	return 0;
}

int main()
{
	srand((unsigned int)time(NULL));	
	HANDLE map_mutex, ready_mutex, time_semaphore, flags_mutex;
	map_mutex = CreateMutex(NULL, FALSE, NULL);
	ready_mutex = CreateMutex(NULL, FALSE, NULL);
	time_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	flags_mutex = CreateMutex(NULL, FALSE, NULL);
	Map_type map;
	int flags[NUMBER_OF_CLIENTS];
	read_BMP("labyrinth.bmp", &map);
	set_exit(&map);
	set_treasures(&map);
	map.skills_number = 0;
	map.time = -1;
	int everybody_ready = 0;
	int everybody_left = 0;
	Thread_args *thread_args;
	Player_type players[NUMBER_OF_CLIENTS];
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++) 
	{
		flags[i] = 0;
		players[i].connected = 0;
		strcpy(players[i].nick, " ");
		players[i].ready = 0;
		players[i].x = 0;
		players[i].y = 0;
		players[i].points = 0;
		players[i].skill = -1;
		players[i].has_left = 0;
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
	thread_args->everybody_left = &everybody_left;
	thread_args->map_mutex = map_mutex;
	thread_args->ready_mutex = ready_mutex;
	thread_args->time_semaphore = time_semaphore;
	thread_args->map = &map;
	thread_args->flags_mutex = flags_mutex;
	thread_args->flags = flags;
	CreateThread(NULL, 0, skills_thread, (void*)thread_args, 0, &id);
	thread_args = (Thread_args*)malloc(sizeof(Thread_args));
	thread_args->everybody_ready = &everybody_ready;
	thread_args->everybody_left = &everybody_left;
	thread_args->map_mutex = map_mutex;
	thread_args->ready_mutex = ready_mutex;
	thread_args->time_semaphore = time_semaphore;
	thread_args->map = &map;
	thread_args->flags_mutex = flags_mutex;
	thread_args->flags = flags;
	CreateThread(NULL, 0, time_thread, (void*)thread_args, 0, &id);
	while (1)
	{
		lenc = sizeof(sc);
		si = accept(s, (struct sockaddr FAR*) &sc, &lenc);
		printf("Client accepted\n");
		
		thread_args = (Thread_args*)malloc(sizeof(Thread_args));
		thread_args->everybody_ready = &everybody_ready;
		thread_args->everybody_left = &everybody_left;
		thread_args->map_mutex = map_mutex;
		thread_args->ready_mutex = ready_mutex;
		thread_args->time_semaphore = time_semaphore;
		thread_args->flags_mutex = flags_mutex;
		thread_args->flags = flags;
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