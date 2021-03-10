#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define HOLD_CMD "HLD"
#define TURN_CMD "TRN"
#define UPDATE_CMD "UPD"
#define COUNT_CMD "CNT"
#define INVALID_CMD "INV"
#define WIN_CMD "WIN"
#define LOSE_CMD "LSE"
#define DRAW_CMD "DRW"
#define WAIT_CMD "WAT"
#define START_CMD "SRT"

void error(const char *msg)
{
    printf("Either the server shut down or the other player disconnected.\nGame over.\n");
    exit(0);
}

void recv_msg(int sockfd, char *msg)
{
    memset(msg, 0, 4);
    int n = read(sockfd, msg, 3);

    if (n < 0 || n != 3)
        error("ERROR reading message from server socket.");
}

int recv_int(int sockfd)
{
    int msg = 0;
    int n = read(sockfd, &msg, sizeof(int));

    if (n < 0 || n != sizeof(int))
        error("ERROR reading int from server socket");

    return msg;
}

void writeServer(int sockfd, int msg)
{
    int n = write(sockfd, &msg, sizeof(int));
    if (n < 0)
        error("ERROR writing int to server socket");
}

int connect_to_server(char *hostname, int portno)
{
    struct sockaddr_in ADDRESS_SERVER;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("Cannot create socket");

    server = gethostbyname(hostname);

    if (server == NULL)
    {
        fprintf(stderr, "no such host\n");
        exit(0);
    }

    memset(&ADDRESS_SERVER, 0, sizeof(ADDRESS_SERVER));

    ADDRESS_SERVER.sin_family = AF_INET;
    memmove(server->h_addr, &ADDRESS_SERVER.sin_addr.s_addr, server->h_length);
    ADDRESS_SERVER.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&ADDRESS_SERVER, sizeof(ADDRESS_SERVER)) < 0)
        perror("Error at connect");

    return sockfd;
}

void drawBoard(char board[][3])
{
    printf(" %c | %c | %c \n-----------\n %c | %c | %c \n-----------\n %c | %c | %c \n",
           board[0][0], board[0][1], board[0][2],
           board[1][0], board[1][1], board[1][2],
           board[2][0], board[2][1], board[2][2]);
}

void take_turn(int sockfd)
{
    char BUFF_STRING[10];

    while (1)
    {
        printf("Enter 0-8 to make a move, or 9 for number of active players: ");
        fgets(BUFF_STRING, 10, stdin);
        int move = BUFF_STRING[0] - '0';
        if (move <= 9 && move >= 0)
        {
            printf("\n");

            writeServer(sockfd, move);
            break;
        }
        else
            printf("\nInvalid input. Try again.\n");
    }
}

void UPDATE_GET(int sockfd, char PLACE_BORAD[][3])
{
    int player_id = recv_int(sockfd);
    int move = recv_int(sockfd);

    PLACE_BORAD[move / 3][move % 3] = player_id ? 'X' : 'O';
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    int sockfd = connect_to_server(argv[1], atoi(argv[2]));

    int id = recv_int(sockfd);

    char msg[4];
    char PLACE_BORAD[3][3] = {{' ', ' ', ' '},
                              {' ', ' ', ' '},
                              {' ', ' ', ' '}};

    do
    {
        recv_msg(sockfd, msg);
        if (!strcmp(msg, HOLD_CMD))
            printf("Waiting for a second player...\n");
    } while (strcmp(msg, START_CMD));

    printf("Game started!\n");
    printf("Your are %c's\n", id ? 'X' : 'O');

    drawBoard(PLACE_BORAD);

    while (1)
    {
        recv_msg(sockfd, msg);

        if (!strcmp(msg, TURN_CMD))
        {
            printf("Your move...\n");
            take_turn(sockfd);
        }
        else if (!strcmp(msg, INVALID_CMD))
        { /* Move was invalid. Note that a "TRN" message will always follow an "INV" message, so we will end up at the above case in the next iteration. */
            printf("That position has already been played. Try again.\n");
        }
        else if (!strcmp(msg, COUNT_CMD))
        { /* Server is sending the number of active players. Note that a "TRN" message will always follow a "CNT" message. */
            int num_players = recv_int(sockfd);
            printf("There are currently %d active players.\n", num_players);
        }
        else if (!strcmp(msg, UPDATE_CMD))
        { /* Server is sending a game PLACE_BORAD update. */
            UPDATE_GET(sockfd, PLACE_BORAD);
            drawBoard(PLACE_BORAD);
        }
        else if (!strcmp(msg, WAIT_CMD))
        { /* Wait for other player to take a turn. */
            printf("Waiting for other players move...\n");
        }
        else if (!strcmp(msg, WIN_CMD))
        { /* Winner. */
            printf("You win!\n");
            break;
        }
        else if (!strcmp(msg, LOSE_CMD))
        { /* Loser. */
            printf("You lost.\n");
            break;
        }
        else if (!strcmp(msg, DRAW_CMD))
        { /* Game is a draw. */
            printf("Draw.\n");
            break;
        }
        else // this should never run
            error("Unknown message.");
    }

    printf("Game over.\n");

    /* Close server socket and exit. */
    close(sockfd);
    return 0;
}