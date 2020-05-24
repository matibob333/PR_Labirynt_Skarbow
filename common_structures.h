<<<<<<< HEAD
#pragma once
#define NUMBER_OF_CLIENTS 4
#define PORT 7777
#define SIZE_OF_DATA 2048
#define STRING_LENGTH 20
#define SPEED 4
#define NEW_SPEED 8
#define TEXTURE_SIZE 16
#define LOCALHOST "127.0.0.1"
#define NUMBER_OF_TREASURES 16
#define TREASURE_OFFSET 16
#define NUMBER_OF_SKILLS 4
#define SKILL_OFFSET 50
#define WALL 0
#define FLOOR 1
#define EXIT 2
=======
#define NUMBER_OF_CLIENTS 4
#define PORT 7777
#define SIZE_OF_DATA 1024
#define STRING_LENGTH 20
#define LOCALHOST "127.0.0.1"
>>>>>>> devel
#include<winsock.h>

//struktura danych gracza
typedef struct Player_type
{
	//składowa pozioma pozycji gracza
	int x;
	//składowa pionowa pozycji gracza
	int y;
	//punkty gracza
	int points;
	//nick gracza
	char nick[STRING_LENGTH];
	//gotowość gracza
	int ready;
	//gniazdo danego gracza do obsługi
	SOCKET socket;
	//czy gracz jest połączony
	int connected;
<<<<<<< HEAD
	// skarby zebrane przez gracza
	int treasures[NUMBER_OF_TREASURES];
	// najważniejszy skarb gracza
	int important_treasure;
	//posiadana umiejetnosc
	int skill;
	//zamrożenie gracza
	int frozen;
	//przyspieszenie gracza
	int speed;
	// czy zakonczyl grę
	int has_left;
=======
>>>>>>> devel
}Player_type;

//struktura mapy
typedef struct Map_type
{
	//rozmiar mapy
	int size;
	//tablica graczy
	Player_type* players;
	//dane labiryntu
	unsigned char** labyrinth;
<<<<<<< HEAD
	// obecna liczba umiętności
	int skills_number;
	//mierzony czas
	int time;
}Map_type;

//struktura danych przesyłanych do klienta
typedef struct Thread_args
{
	//numer danego gracza
	int player_number;
	//wskaźnik na wspólną mapę
	Map_type* map;
	//wskaznik na zmienna, czy gracze sa gotowi
	int* everybody_ready;
	//wskaznik na zmienna, czy gracze wyszli z labiryntu
	int* everybody_left;
	//uchwyt mutexa mapy
	HANDLE map_mutex;
	//uchwyt mutexa gotowości graczy
	HANDLE ready_mutex;
	//uchwyt semafora czasu
	HANDLE time_semaphore;
}Thread_args;
=======
}Map_type;
>>>>>>> devel
