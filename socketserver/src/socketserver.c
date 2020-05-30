/*
 ============================================================================
 Name        : socketserver.c
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
#include <arpa/inet.h>

#include <string.h>

#include <errno.h>

extern int errno;

#define RD_END_PIPE 0	/* Read-end of the pipe - Children send the data to the parent, Children should close fd0*/
#define WR_END_PIPE 1	/* Write-end of the pipe - Parent receives data from children, Parent should close fd1*/


int main()
{
	pid_t pidClient;

	pid_t pidClientSender;
	pid_t pidClientReceiver;
	pid_t pidClientReceiverFromStdin;
	pid_t pidClientReceiveFromOthersClients;

	int errnum;

	char server_message[256] = "Hello ";
	char client_message[256];
	char client_message_parent[256];
	char parent_message_client[256];

	int  pipeClientStdinToSender[2];

	int server_socket, new_client_socket;
	struct sockaddr_in server_address;
	struct sockaddr_in new_client_address;

	int new_client_address_len;

	memset(&server_address, '\0', sizeof(server_address));
	memset(&new_client_address, '\0', sizeof(new_client_address));


	server_socket = socket(	AF_INET,
							SOCK_STREAM,
							0);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	bind(	server_socket,
			(struct sockaddr*) &server_address,
			sizeof(server_address));

	listen(	server_socket,
			5);

	// have to accept several incoming clients, I use fork

	while(1)
	{
		new_client_address_len = sizeof(new_client_address);

		new_client_socket = accept(	server_socket,
									(struct socketadd *)&new_client_address,
									&(new_client_address_len));

		if(new_client_socket < 0)
		{
			/* use errno to handle a connection error on a debug phase */
			errnum = errno;
			printf("Value of errno: %d\n", errnum);

			/* error receivnig new incomming clients */
			/*TODO not close everything haha */
			printf("Error handling new clients. Everything shutdown\n");
			close(server_socket);
			exit(1);
		}

		/* make a fork. return 0 to the child - return childPid to the parent */
		pidClient = fork();

		if(pidClient == 0)
		{
			/* inside the child - pidClient has a 0*/

			/* receive the client nickname */
			recv(	new_client_socket,
					&client_message,
					sizeof(client_message),
					0);

			/* Print all information server side */
			printf("	New client information:\n "
					"	name: %s\n "
					"	address: %s\n "
					"	port: %i\n",
					client_message,
					inet_ntoa(new_client_address.sin_addr),
					ntohs(new_client_address.sin_port));

			// prepare welcome message to client and send it
			strcat(server_message, client_message);
			strcat(server_message, " to the server");

			// send welcome msg to the new client
			send(	new_client_socket,
					server_message,
					sizeof(server_message),
					0);

			/* Close the server port because is already in use by the parent. Its not
			 * useful anymore on the child */
			close(server_socket);

			/* I need in total three process to control the client
			 * 1 - Receive information from other child clients (what others have writing
			 * 2 - Receive information from current child client (keyboard)
			 * 3 - Send both above to all clients */

			/* pidClientSender stores 0 the first child of the fork.
			 * Will use it to send to all clients all messages */

			pidClientSender = getpid();

			pipe(pipeClientStdinToSender);

			pidClientReceiver = fork();

			if(pidClientReceiver == 0)
			{
				/* Im in the Receiver, here I will have to receive from
				 * Stdin
				 * Other clients */

				/* Close the reading pipe */
				close(pipeClientStdinToSender[RD_END_PIPE]);

				while(1)
				{
					/* Empty the buffer */
					memset(client_message, '\0', sizeof(client_message));

					/* print whatever the client sends */
					recv(new_client_socket, client_message, sizeof(client_message), 0);

					/* TODO need a close strg to disconnect */
					/* check if it works properly */
					if(strcmp(client_message, "!exit") == 0)
					{
						/* client send the exit cmd */
						printf("Child %d disconnecting...\n", getpid());
						close(new_client_socket);
						break;
					}
					else
					{
						printf("Child process %d says: %s\n", getpid(), client_message);
						printf("Sending to clients... \n");
						/* Send it also to the this parent the Sender to share with all clients */
						write(pipeClientStdinToSender[WR_END_PIPE], client_message, (strlen(client_message)+1));

					}

				}

			}
			else
			{

				/* Im in the parent, the one that I will use to send all information */
				/* send received and write msg to all clients */

				pidClientSender = getpid();
				close(pipeClientStdinToSender[WR_END_PIPE]);

				while(1)
				{
					/* Empty the buffer */
					memset(parent_message_client, '\0', sizeof(parent_message_client));

					/* Read in a string from the pipe */
					read(pipeClientStdinToSender[RD_END_PIPE], parent_message_client, sizeof(parent_message_client));


					send(	new_client_socket,
							parent_message_client,
							sizeof(parent_message_client),
							0);

				}


			}

		}
		else
		{
			/* inside the parent - pidClient has the pidChild */
			memset(client_message_parent, '\0', sizeof(client_message_parent));

			printf("A new client connected with pidChild: %d\n", pidClient);
			printf("Waiting for new clients...");

		}

	}

	close(server_socket);
	return 0;

}


