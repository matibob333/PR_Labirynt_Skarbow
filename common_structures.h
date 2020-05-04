#define NUMBER_OF_CLIENTS 4
#define PORT 420
#define SIZE_OF_DATA 1024
#define STRING_LENGTH 20
#define LOCALHOST "127.0.0.1"

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
}Player_type;

//struktura mapy
typedef struct Map_type
{
	//rozmiar mapy
	int size;
	//tablica graczy
	Player_type* players;
}Map_type;