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

int get_player_move(int cli_sockfd)
{
    write_client_msg(cli_sockfd, TURN_CMD);

    return recv_int(cli_sockfd);
}

int check_move(char board[][3], int move, int player_id)
{
    if ((move == 9) || (board[move / 3][move % 3] == ' '))
    {
        return 1;
    }
    return 0;
}

/* Updates the board with a new move. */
void update_board(char board[][3], int move, int player_id)
{
    board[move / 3][move % 3] = player_id ? 'X' : 'O';
}

void draw_board(char board[][3])
{
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
}

void *run_game(void *thread_data)
{
    int *cli_sockfd = (int *)thread_data; /* Client sockets. */
    char board[3][3] = {{' ', ' ', ' '},  /* Game Board */
                        {' ', ' ', ' '},
                        {' ', ' ', ' '}};

    printf("Game on!\n");

    write_clients_msg(cli_sockfd, START_CMD);

    draw_board(board);

    int prev_player_turn = 1;
    int player_turn = 0;
    int game_over = 0;
    int turn_count = 0;
    while (!game_over)
    {
        if (prev_player_turn != player_turn)
            write_client_msg(cli_sockfd[(player_turn + 1) % 2], WAIT_CMD);

        int valid = 0;
        int move = 0;
        while (!valid)
        {
            move = get_player_move(cli_sockfd[player_turn]);
            if (move == -1)
                break;

            printf("Player %d played position %d\n", player_turn, move);

            valid = check_move(board, move, player_turn);
            if (!valid)
            {
                printf("Move was invalid. Let's try this again...\n");
                write_client_msg(cli_sockfd[player_turn], INVALID_CMD);
            }
        }

        if (move == -1)
        {
            printf("Player disconnected.\n");
            break;
        }
        else if (move == 9)
        {
            prev_player_turn = player_turn;
            send_COUNT_OF_PLAYER(cli_sockfd[player_turn]);
        }
        else
        {
            update_board(board, move, player_turn);
            send_update(cli_sockfd, move, player_turn);

            draw_board(board);

            game_over = check_board(board, move);

            if (game_over == 1)
            {
                write_client_msg(cli_sockfd[player_turn], WIN_CMD);
                write_client_msg(cli_sockfd[(player_turn + 1) % 2], LOSE_CMD);
                printf("Player %d won.\n", player_turn);
            }
            else if (turn_count == 8)
            {
                printf("Draw.\n");
                write_clients_msg(cli_sockfd, DRAW_CMD);
                game_over = 1;
            }

            prev_player_turn = player_turn;
            player_turn = (player_turn + 1) % 2;
            turn_count++;
        }
    }

    printf("Game over.\n");

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    pthread_mutex_lock(&COUNT_MUTEX);
    nr_players-=2;
    printf("Number of players is now %d.", nr_players);
    pthread_mutex_unlock(&COUNT_MUTEX);

    free(cli_sockfd);

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