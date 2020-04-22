#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>

#define PORT 420

#pragma comment(lib, "Ws2_32.lib")

int main() {
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
	result = listen(s, 4);

	SOCKET si;
	struct sockaddr_in sc;
	int lenc;
	char buf[20] = "PONG";
	while (1)
	{
		lenc = sizeof(sc);
		si = accept(s, (struct sockaddr FAR*) &sc, &lenc);
		printf("Client accepted\n");
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
		if (recv(si, buf, 20, 0) > 0)
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