#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int nr_players = 0;
pthread_mutex_t COUNT_MUTEX;
int setup_listener(int portno)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening listener socket.");

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR binding listener socket.");

    printf("Server started on port : %d\n", atoi(portno));
    return sockfd;
}

void *run_game(void *thread_data)
{
    printf("Separate thread for X USer");
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    int lis_sockfd = setup_listener(atoi(argv[1]));

    while (1)
    {

        int result = pthread_create(&thread, NULL, run_game, (void *)cli_sockfd);
    }

    close(lis_sockfd);

    pthread_mutex_destroy(&COUNT_MUTEX);
    pthread_exit(NULL);
}