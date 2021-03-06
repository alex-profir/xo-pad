#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
	int sock = 0, valread;
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
	char buff[200];
	char name[100];
	printf("Please enter your name ");
	scanf("%s", name);
	send(sock, name, strlen(name), 0);
	while (1)
	{
		printf("Send a message to the server ... ");
		// scanf("%s", buff);
		fgets(buff, sizeof(buff), stdin);
		buff[strlen(buff) - 1] = '\0';
		send(sock, buff, strlen(buff), 0);
		printf("%s message sent\n", buff);
	}
	valread = read(sock, buffer, 1024);
	printf("%s\n", buffer);
	return 0;
}
