#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "common_structures.h"

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
}Thread_args;

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

void serialize_map_fully(char* data, Map_type* map, int *everybody_ready)
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
	}
	memcpy(data + position, everybody_ready, sizeof(int));
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
	while (!(*(arguments->everybody_ready)))
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
			//gracz do³¹cza do gry
			if (strcmp(buf, "connect")==0)
			{
				recv(client_socket, buf, STRING_LENGTH, 0);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].points = 0;
					int x = rand() % arguments->map->size;
					int y = rand() % arguments->map->size;
					arguments->map->players[player_number].x = x;
					arguments->map->players[player_number].y = y;
					arguments->map->players[player_number].connected = 1;
					arguments->map->players[player_number].ready = 0;
					strcpy(arguments->map->players[player_number].nick, buf);
				}
				ReleaseMutex(arguments->map_mutex);
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
				}
				ReleaseMutex(arguments->ready_mutex);
				*(arguments->everybody_ready) = rd;
				strcpy(buf, "full_map");
				send(client_socket, buf, STRING_LENGTH, 0);
				char* data = (char*)malloc(SIZE_OF_DATA);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					serialize_map_fully(data, arguments->map, arguments->everybody_ready);
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
	if (WaitForSingleObject(arguments->ready_mutex, INFINITE) == WAIT_OBJECT_0)
	{
		*(arguments->everybody_ready) = 0;
	}
	ReleaseMutex(arguments->ready_mutex);
	while (!lost_client)
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
			if (strcmp(buf, "up") == 0)
			{
				printf("Player number %d sent GURA\n", player_number);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].y--;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			else if (strcmp(buf, "down") == 0)
			{
				printf("Player number %d sent DUU\n", player_number);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].y++;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			else if (strcmp(buf, "right") == 0)
			{
				printf("Player number %d sent PRAWO\n", player_number);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].x++;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			else if (strcmp(buf, "left") == 0)
			{
				printf("Player number %d sent LEWO\n", player_number);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					arguments->map->players[player_number].x--;
				}
				ReleaseMutex(arguments->map_mutex);
			}
			else if (strcmp(buf, "ping") == 0)
			{
				strcpy(buf, "pong");
				send(client_socket, buf, STRING_LENGTH, 0);
				strcpy(buf, "full_map");
				send(client_socket, buf, STRING_LENGTH, 0);
				char* data = (char*)malloc(SIZE_OF_DATA);
				if (WaitForSingleObject(arguments->map_mutex, INFINITE) == WAIT_OBJECT_0)
				{
					serialize_map_fully(data, arguments->map, arguments->everybody_ready);
				}
				ReleaseMutex(arguments->map_mutex);
				send(client_socket, data, SIZE_OF_DATA, 0);
				free(data);
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

int main() {
	HANDLE map_mutex, ready_mutex;
	map_mutex = CreateMutex(NULL, FALSE, NULL);
	ready_mutex = CreateMutex(NULL, FALSE, NULL);
	Map_type map;
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
	while (1)
	{
		lenc = sizeof(sc);
		si = accept(s, (struct sockaddr FAR*) &sc, &lenc);
		printf("Client accepted\n");
		
		thread_args = (Thread_args*)malloc(sizeof(Thread_args));
		thread_args->everybody_ready = &everybody_ready;
		thread_args->map_mutex = map_mutex;
		thread_args->ready_mutex = ready_mutex;
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
	closesocket(si);
	WSACleanup();
	return 0;
}