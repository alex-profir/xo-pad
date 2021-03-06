#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_NR_PLAYERS 252
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

int nr_players = 0;
pthread_mutex_t COUNT_MUTEX;

void error(const char *msg)
{
	perror(msg);
	pthread_exit(NULL);
}

int recv_int(int cli_sockfd)
{
	int msg = 0;
	int n = read(cli_sockfd, &msg, sizeof(int));

	if (n < 0 || n != sizeof(int))
		return -1;

	return msg;
}

void write_client_msg(int cli_sockfd, char *msg)
{
	int n = write(cli_sockfd, msg, strlen(msg));
	if (n < 0)
		error("ERROR writing msg to client socket");
}

void write_client_int(int cli_sockfd, int msg)
{
	int n = write(cli_sockfd, &msg, sizeof(int));
	if (n < 0)
		error("ERROR writing int to client socket");
}

void write_clients_msg(int *cli_sockfd, char *msg)
{
	write_client_msg(cli_sockfd[0], msg);
	write_client_msg(cli_sockfd[1], msg);
}

void write_clients_int(int *cli_sockfd, int msg)
{
	write_client_int(cli_sockfd[0], msg);
	write_client_int(cli_sockfd[1], msg);
}

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
	printf("Server started on port %d\n", portno);
	return sockfd;
}

void waitClient(int lis_sockfd, int *cli_sockfd)
{
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	int nConnections = 0;
	while (nConnections < 2)
	{
		listen(lis_sockfd, 253 - nr_players);

		memset(&cli_addr, 0, sizeof(cli_addr));

		clilen = sizeof(cli_addr);

		cli_sockfd[nConnections] = accept(lis_sockfd, (struct sockaddr *)&cli_addr, &clilen);

		if (cli_sockfd[nConnections] < 0)
			error("ERROR accepting a connection from a client.");

		write(cli_sockfd[nConnections], &nConnections, sizeof(int));

		pthread_mutex_lock(&COUNT_MUTEX);
		nr_players++;
		printf("Number of players is now %d.\n", nr_players);
		pthread_mutex_unlock(&COUNT_MUTEX);

		if (nConnections == 0)
		{
			write_client_msg(cli_sockfd[0], HOLD_CMD);
		}

		nConnections++;
	}
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

void send_update(int *cli_sockfd, int move, int player_id)
{
	write_clients_msg(cli_sockfd, UPDATE_CMD);

	write_clients_int(cli_sockfd, player_id);

	write_clients_int(cli_sockfd, move);
}

void send_COUNT_OF_PLAYER(int cli_sockfd)
{
	write_client_msg(cli_sockfd, COUNT_CMD);
	write_client_int(cli_sockfd, nr_players);
}

int check_board(char board[][3], int last_move)
{
	int row = last_move / 3;
	int col = last_move % 3;

	if (board[row][0] == board[row][1] && board[row][1] == board[row][2])
	{
		return 1;
	}
	else if (board[0][col] == board[1][col] && board[1][col] == board[2][col])
	{
		return 1;
	}
	else if (!(last_move % 2))
	{
		if ((last_move == 0 || last_move == 4 || last_move == 8) && (board[1][1] == board[0][0] && board[1][1] == board[2][2]))
		{
			return 1;
		}
		if ((last_move == 2 || last_move == 4 || last_move == 6) && (board[1][1] == board[0][2] && board[1][1] == board[2][0]))
		{
			return 1;
		}
	}
	return 0;
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
	nr_players--;
	printf("Number of players is now %d.", nr_players);
	nr_players--;
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
	pthread_mutex_init(&COUNT_MUTEX, NULL);

	while (1)
	{
		if (nr_players <= MAX_NR_PLAYERS)
		{
			int *cli_sockfd = (int *)malloc(2 * sizeof(int));
			memset(cli_sockfd, 0, 2 * sizeof(int));

			waitClient(lis_sockfd, cli_sockfd);

			pthread_t thread;
			int result = pthread_create(&thread, NULL, run_game, (void *)cli_sockfd);
			if (result)
			{
				printf("Thread creation failed with return code %d\n", result);
				exit(-1);
			}
		}
		else
		{
			printf("Max nr of players exceeded \n");
		}
	}

	close(lis_sockfd);

	pthread_mutex_destroy(&COUNT_MUTEX);
	pthread_exit(NULL);
}
