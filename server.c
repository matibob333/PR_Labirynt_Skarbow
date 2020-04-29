#define _CRT_SECURE_NO_WARNINGS
#define NUMBER_OF_CLIENTS 4
#define PORT 420
#define SIZE_OF_DATA 1024
#define STRING_LENGTH 20

#include<windows.h>
#include<stdio.h>
#include<time.h>

#pragma comment(lib, "Ws2_32.lib")

//struktura danych gracza
typedef struct Player_type
{
	//sk³adowa pozioma pozycji gracza
	int x;
	//sk³adowa pionowa pozycji gracza
	int y;
	//punkty gracza
	int points;
	//nick gracza
	char nick[STRING_LENGTH];
}Player_type;

//struktura mapy
typedef struct Map_type
{
	//liczba graczy
	int number_of_players;
	//tablica graczy
	Player_type* players;
}Map_type;

//struktura danych przesy³anych do klienta
typedef struct Thread_args
{
	//gniazdo danego gracza do obs³ugi
	SOCKET socket;
	//numer danego gracza
	int player_number;
	//wskaŸnik na wspóln¹ mapê
	Map_type* map;
}Thread_args;

//serializacja danych z mapy do tablicy bajtów w celu przes³ania
void serialize_map(char* data, Map_type* map)
{
	char int_holder[sizeof(int)];
	strcpy(data, "");
	for (int i = 0; i < map->number_of_players; i++)
	{
		memcpy(int_holder, &(map->players[i].x), sizeof(int));
		strcat(data, int_holder);
		memcpy(int_holder, &(map->players[i].y), sizeof(int));
		strcat(data, int_holder);
		memcpy(int_holder, &(map->players[i].points), sizeof(int));
		strcat(data, int_holder);
	}
}

DWORD WINAPI client_thread(void* args)
{
	char buf[STRING_LENGTH];
	int lost_client = 0;
	Thread_args* arguments = (Thread_args*)args;
	int byte_no = SIZE_OF_DATA;
	while (!lost_client)
	{
		if (recv(arguments->socket, buf, STRING_LENGTH, 0) > 0)
		{
			if (strcmp(buf, "up") == 0)
			{
				printf("Player number %d sent GURA\n", arguments->player_number);
				arguments->map->players[arguments->player_number].y--;
			}
			else if (strcmp(buf, "down") == 0)
			{
				printf("Player number %d sent DUU\n", arguments->player_number);
				arguments->map->players[arguments->player_number].y++;
			}
			else if (strcmp(buf, "right") == 0)
			{
				printf("Player number %d sent PRAWO\n", arguments->player_number);
				arguments->map->players[arguments->player_number].x++;
			}
			else if (strcmp(buf, "left") == 0)
			{
				printf("Player number %d sent LEWO\n", arguments->player_number);
				arguments->map->players[arguments->player_number].x--;
			}
			else if (strcmp(buf, "ping") == 0)
			{
				strcpy(buf, "PONG");
				send(arguments->socket, buf, STRING_LENGTH, 0);
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
		send(arguments->socket, data, byte_no, 0);
		free(data);
	}
	free(arguments);
	return 0;
}

int main() {
	Map_type map;
	map.number_of_players = 0;
	Thread_args *thread_args;
	Player_type players[NUMBER_OF_CLIENTS];	map.players = players;
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
		thread_args->socket = si;
		players[map.number_of_players].x=0;
		players[map.number_of_players].y=0;
		players[map.number_of_players].points=0;
		strcpy(players[map.number_of_players].nick,"BOB");
		map.number_of_players++;
		thread_args->player_number = map.number_of_players;
		thread_args->map = &map;
		CreateThread(NULL, 0, client_thread, (void*)thread_args,0, &id);
		//trzeba dac do nowego watku i odpowienio konczyc petle
		/*while (1)
		{
			if (recv(si, buf, 30, 0) > 0) {
				buf[5] = '\0';
				printf("Received %s from client, sending PONG\n", buf);
			}
			else {
				printf("ERROR\n");
			}
			strcpy(buf, "PONG");
			send(si, buf, 30, 0);
		}*/
		if (recv(si, buf, STRING_LENGTH, 0) > 0)
		{
			if (strcmp(buf, "up") == 0)
			{
				printf("GURA\n");
			}
			else if (strcmp(buf, "down") == 0)
			{
				printf("DUU\n");
			}
			else if (strcmp(buf, "right") == 0)
			{
				printf("PRAWO\n");
			}
			else if (strcmp(buf, "left") == 0)
			{
				printf("LEWO\n");
			}
			else
			{
				printf("NIEZROZUMIALE\n");
			}
		}
		else
		{
			printf("ERROR\n");
		}
	}
	closesocket(si);
	WSACleanup();
	return 0;
}