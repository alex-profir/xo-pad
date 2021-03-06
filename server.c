#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
void *serverthread(void *parm);

#define PORT 8080
#define QLEN 10		  /* size of request queue */
#define MAXRCVLEN 200 /*Maximum size of buffer*/
#define STRLEN 200	  /*Maximum String length*/
pthread_mutex_t mut;  /* Mutex to prevent race conditions. */

int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";
	pthread_t tid; /* variable to hold thread ID */

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
								 (socklen_t *)&addrlen)) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		pthread_create(&tid, NULL, serverthread, (void *)new_socket);
	}
	close(server_fd);
	return 0;
}
void *serverthread(void *parm)
{
	int tsd, len;
	tsd = (int)parm; /*Thread Socket descriptor*/

	printf("Connected with %d\n", tsd);
	char ip[INET_ADDRSTRLEN]; /*Char array to store client's IP address*/
	struct sockaddr_in peeraddr;
	socklen_t peeraddrlen = sizeof(peeraddr);
	getpeername(tsd, &peeraddr, &peeraddrlen);												 /*Retrives address of the peer to which a socket is connected*/
	inet_ntop(AF_INET, &(peeraddr.sin_addr), ip, INET_ADDRSTRLEN); /*Binary to text string*/ /*Retriving IP addrees of client and converting 
	it to text and storing it in IP char array*/

	char buf[MAXRCVLEN + 1]; /* buffer for data exchange */
	char name[STRLEN + 1];	 /* Variable to store current client's name. */
	recv(tsd, buf, MAXRCVLEN, 0);
	strcpy(name, buf);
	while (len = recv(tsd, buf, MAXRCVLEN, 0))
	{
		buf[len] = '\0';
		char arg1[STRLEN]; //, arg2[STRLEN];
		int n = sscanf(buf, "%s", arg1);
		printf("%s:%s\n", name, arg1);
	}
	printf("Connection lost with %d\n", tsd);
	pthread_mutex_unlock(&mut);
	close(tsd);
	pthread_exit(0);
}