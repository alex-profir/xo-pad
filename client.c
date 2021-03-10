#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

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

	/* The game has begun. */
	printf("Game on!\n");
	printf("Your are %c's\n", id ? 'X' : 'O');

	draw_PLACE_BORAD(PLACE_BORAD);

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
			draw_PLACE_BORAD(PLACE_BORAD);
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