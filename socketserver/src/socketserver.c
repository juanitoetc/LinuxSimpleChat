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

extern int errno ;


int main()
{
	pid_t pidRtn;
	int errnum;

	char server_message[256] = "Hello from the server";
	char client_message[256];

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

		/* new client successfully connected */
		printf("New client information: add:%s - port:%i", inet_ntoa(new_client_address.sin_addr), ntohs(new_client_address.sin_port));

		/* make a fork. return 0 to the child - return childPid to the parent */
		pidRtn = fork();

		if(pidRtn == 0)
		{
			/* inside the child - pidRtn has a 0*/
			// send welcome msg to the new client
			send(	new_client_socket,
					server_message,
					sizeof(server_message),
					0);

			/* close the port because is already in use and not available anymore */
			close(server_socket);

			while(1)
			{
				/* print whatever the client sends */
				recv(new_client_socket, client_message, sizeof(client_message), 0);

				/* TODO need a close strg to disconnect */
				/* check if it works properly */
				if(strcmp(client_message, "!exit") == 0)
				{
					/* client send the exit cmd */
					printf("Child %d disconnecting...", getpid());
					close(new_client_socket);
					break;
				}


				printf("child process %d says: %s\n", getpid(), client_message);

				/* TODO send it to all clients */
			}
		}
		else
		{
			/* inside the parent - pidRtn has the pidChild */
			printf("A new client connected with pidChild: %d\n", pidRtn);
			printf("Waiting for new clients...");
		}

	}

	close(server_socket);
	return 0;

}


