#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "../common/common_structures.h"

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

}Thread_args;

//serializacja danych z mapy do tablicy bajtów w celu przes³ania
void serialize_map(char* data, Map_type* map)
{
	char int_holder[sizeof(int)];
	strcpy(data, "");
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		if (map->players[i].connected)
		{
			memcpy(int_holder, &(map->players[i].x), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(map->players[i].y), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(map->players[i].points), sizeof(int));
			strcat(data, int_holder);
		}
	}
}

void serialize_map_fully(char* data, Map_type* map)
{
	char int_holder[sizeof(int)];
	strcpy(data, "");

	for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		if (map->players[i].connected)
		{
			memcpy(int_holder, &(map->players[i].x), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(map->players[i].y), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(map->players[i].ready), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(map->players[i].connected), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(map->players[i].points), sizeof(int));
			strcat(data, int_holder);
			memcpy(int_holder, &(strlen(map->players[i].nick), sizeof(int));
			strcat(data, int_holder);
			strcat(data, map->players[i].nick);
		}
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
	srand(time(NULL) * GetCurrentThreadId());
	while (!(*(arguments->everybody_ready)))
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
			//gracz do³¹cza do gry
			if (strcmp(buf, "connect"))
			{
				recv(client_socket, buf, STRING_LENGTH, 0);
				arguments->map->players[player_number].points = 0;
				int x = rand() % argumets->map->size;
				int y = rand() % argumets->map->size;
				arguments->map->players[player_number].x = x;
				arguments->map->players[player_number].y = y;
				arguments->map->players[player_number].connected = 1;
				arguments->map->players[player_number].ready = 1;
				strcpy(arguments->map->players[player_number].nick, buf);
			}
			//gracz zg³asza gotowoœæ
			else if (strcmp(buf, "ready"))
			{
				arguments->map->players[player_number].ready = 1;
			}
			//gracz zg³asza brak gotowoœci
			else if (strcmp(buf, "not_ready"))
			{
				arguments->map->players[player_number].ready = 0;
			}
			//gracz od³¹cza siê od rozrywki
			else if (strcmp(buf, "disconnect"))
			{
				arguments->map->players[number_of_player].connected = 0;
				//zakoñczenie w¹tku
				free(arguments);
				return 0;
			}
			else if (strcmp(buf, "ping"))
			{
				
			}
			else
			{
				//TODO
			}
		}
		int rd = 1;
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
	}
	while (!lost_client)
	{
		if (recv(client_socket, buf, STRING_LENGTH, 0) > 0)
		{
			if (strcmp(buf, "up") == 0)
			{
				printf("Player number %d sent GURA\n", player_number);
				arguments->map->players[player_number].y--;
			}
			else if (strcmp(buf, "down") == 0)
			{
				printf("Player number %d sent DUU\n", player_number);
				arguments->map->players[player_number].y++;
			}
			else if (strcmp(buf, "right") == 0)
			{
				printf("Player number %d sent PRAWO\n", player_number);
				arguments->map->players[player_number].x++;
			}
			else if (strcmp(buf, "left") == 0)
			{
				printf("Player number %d sent LEWO\n", player_number);
				arguments->map->players[player_number].x--;
			}
			else if (strcmp(buf, "ping") == 0)
			{
				strcpy(buf, "PONG");
				send(client_socket, buf, STRING_LENGTH, 0);
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
		//alokowana pamiêæ do danych do przesy³ania - zak³adamy nieprzekroczenie 1kB danych - mo¿liwe do zmiany!
		char* data = (char*)malloc(byte_no);
		serialize_map(data, arguments->map);
		send(client_socket, data, byte_no, 0);
		free(data);
	}
	free(arguments);
	return 0;
}

int main() {
	Map_type map;
	int everybody_ready = 0;
	Thread_args *thread_args;
	Player_type players[NUMBER_OF_CLIENTS];
	for (int i = 0; i < NUMBER_OF_CLIENTS; i++) 
	{
		players.connected = 0;
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