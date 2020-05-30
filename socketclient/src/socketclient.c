/*
 ============================================================================
 Name        : socketclient.c
 Author      : juani
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <unistd.h>
#include <string.h>
#include <signal.h>

int main()
{
	int network_socket, connection_status;
	struct sockaddr_in server_address;

	char server_response[256];
	char client_msg[256];
	char nickNameClient[256];
	int strLen = 0;
	char parent_message_client[256];

	pid_t pidClientSide;

	int loop = 0, died = 0, status = 0;

	/* fgets the nickname */
	printf("Ingresa tu nombre de usuario: ");
	memset(nickNameClient, '\0', sizeof(nickNameClient));
	fgets(nickNameClient, sizeof(nickNameClient), stdin);

	/*delete the \n from the input*/
	strLen = strlen(nickNameClient);
	nickNameClient[strLen - 1] = '\0';

	network_socket = socket(AF_INET,
							SOCK_STREAM,
							0);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	printf("Connecting %s to server... \n", nickNameClient);

	connection_status = connect(	network_socket,
									(struct sockadd *) &server_address,
									sizeof(server_address));


	if(connection_status == -1)
	{
		/* Server is shutdown */
		printf("Error connection to the remote socket.\n");
		close(network_socket);
		return 0;
	}

	printf("Connected to server.\n");

	/* First send the name to the server */
	send(	network_socket,
			nickNameClient,
			strlen(nickNameClient),
			0);

	/* Then wait for the welcome message from server */
	recv(	network_socket,
			&server_response,
			sizeof(server_response),
			0);

	printf("%s \n\n", server_response);


	pidClientSide = fork();

	if(pidClientSide == 0)
	{
		/* It is the child - use it to receive messages from parent and print it */

		while(1)
		{
			memset(server_response, '\0', sizeof(server_response));

			recv(	network_socket,
					&server_response,
					sizeof(server_response),
					0);

			printf("Child process says: %s\n", server_response);
		}

	}
	else
	{

		while(1)
		{
			/* It is the parent. Used to scand stdin and send it to server */
			/* scanf a new message */
			fgets(client_msg, sizeof(client_msg), stdin);
			strLen = strlen(client_msg);
			client_msg[strLen - 1] = '\0';

			/* dont send 256 characters only send until EOL */
			send(network_socket, client_msg, strlen(client_msg), 0);

			/* need a cmd to close the connection */
			if(strcmp(client_msg, "!exit") == 0)
			{
				/* client wants to close connection - send special cmd to server */
				printf("Disconnecting...\n");
				close(network_socket);

				/* kill child */
				kill(pidClientSide, SIGTERM);

				died = 0;
				for(loop; (!died && loop < 5) ; ++loop)
				{

				    pid_t id;
				    sleep(1);
				    if (waitpid(pidClientSide, &status, WNOHANG) == pidClientSide)
				    	died = 1;
				}

				if (died == 0)
					kill(pidClientSide, SIGKILL);

				/*exit client parent */
				exit(0);

			}
		}
	}


}
