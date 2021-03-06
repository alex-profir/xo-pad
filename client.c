#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define PORT 8080

void *serverthread(void *parm);

int sock = 0;
pthread_t tid; /*Thread ID*/
int main(int argc, char const *argv[])
{
	int valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	pthread_create(&tid, NULL, serverthread, (void *)&sock);
	char name[100];
	printf("Please enter your name ");
	scanf("%s", name);
	send(sock, name, strlen(name), 0);
	while (1)
	{
		valread = read(sock, buffer, 1024);
		printf("%s\n", buffer);
	}
	return 0;
}
void *serverthread(void *obj)
{
	char buff[200];
	while (1)
	{
		scanf("%s", buff);
		send(sock, buff, strlen(buff), 0);
	}
	pthread_exit(0);
}