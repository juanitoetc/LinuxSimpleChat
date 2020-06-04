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

#include <sys/mman.h>

#include <fcntl.h>


extern int errno;

#define RD_END_PIPE 0	/* Read-end of the pipe - Children send the data to the parent, Children should close fd0*/
#define WR_END_PIPE 1	/* Write-end of the pipe - Parent receives data from children, Parent should close fd1*/

#define MAX_CLIENTS 5
#define MAX_LENGHT_CHAR	256

#define _GNU_SOURCE


/* Structure in the server listener of current connected clients */
typedef struct stClientData
{
	int iBusy;		/* quick way to know which places are being used*/
	struct sockaddr_in stClientAddress;
	int	isocketId;
	int iUserId;
	char chUserName[MAX_LENGHT_CHAR];
	char chMsgBuff[MAX_LENGHT_CHAR];
	int Dirty;

}stClientData;

stClientData stClients[MAX_CLIENTS];


int main()
{
	int flags;

	fd_set status, current;
	pid_t pidClient;

	struct timeval tv;

	pid_t pidClientSender;
	pid_t pidClientReceiver;
	pid_t pidClientReceiverFromStdin;
	pid_t pidClientReceiveFromOthersClients;

	int errnum;
	int i= 0, j = 0, h = 0, k = 0;
	int new = 0;
	int currentSocket;

	char server_message[256] = "Hello ";
	char client_message[256];
	char client_message_parent[256];
	char parent_message_client[256];

	int  pipeClientStdinToSender[2];
	int  pipeUpdateClientsList[2];

	int iRtnAccept;

	int server_socket, new_client_socket;
	struct sockaddr_in server_address;
	struct sockaddr_in new_client_address;

	int new_client_address_len;
	int currentClients = 0;

	memset(&server_address, '\0', sizeof(server_address));
	memset(&new_client_address, '\0', sizeof(new_client_address));

	tv.tv_sec = 3;
	tv.tv_usec = 0;


	server_socket = socket(	AF_INET,
							SOCK_STREAM,
							0);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	bind(	server_socket,
			(struct sockaddr*) &server_address,
			sizeof(server_address));

	listen(	server_socket, MAX_CLIENTS);

	// have to accept several incoming clients,
	// Can't use fork because only the last client connected will have access to the full fd table,
	// previous clients don't get the latest fd

	// I use select

	// initializes a descriptor set fdset	to the null set.
	FD_ZERO(&status);

	// includes a particular descriptor fd in fdset.
	FD_SET(server_socket, &status);

	memset(stClients[0].chMsgBuff, '\0', sizeof(stClients[0].chMsgBuff));
	memset(stClients[0].chUserName, '\0', sizeof(stClients[0].chUserName));
	strcpy(stClients[0].chUserName,"Listener");
	stClients[0].iBusy = 1;
	stClients[0].isocketId = server_socket;
	stClients[0].stClientAddress = server_address;
	stClients[0].iUserId = 0;

	FD_SET(0, &status);

	while(1)
	{
		/* sleep a littlebit */
		sleep(1);

		/* make a copy, every time I call select fd_set is destroyed */
		current = status;

		/* from now current is my complete fd_set */

		/* nfds is the highest-numbered file descriptor in any of the three sets, plus 1 */
		if (select(MAX_CLIENTS +  2, &current, NULL, NULL, &tv) == -1 )
		{
			perror("Select");
			return 0;
		}

		for (j =0; j< MAX_CLIENTS + 2; j++)
		{
			currentSocket = stClients[j].isocketId;

			if (FD_ISSET(currentSocket, &current))
			{
				if (currentSocket == server_socket)
				{
					/* im in the listener */
					/* should accept the new connection and add it to the list */
					memset(client_message, '\0', sizeof(client_message));

					new_client_address_len = sizeof(new_client_address);

					new_client_socket = accept(	currentSocket,
												(struct socketadd *)&new_client_address,
												&new_client_address_len);

					flags = fcntl(new_client_socket, F_GETFL, 0);
					fcntl(new_client_socket, F_SETFL, flags | O_NONBLOCK);

					if (new_client_socket == -1)
					{
						perror ("Couldn't accept connection");
					}
					else if (new_client_socket > MAX_CLIENTS + 2)
					{
						printf ("Unable to accept new connection.\n");
						close(new_client_socket);
					}
					else
					{
						currentClients++;

						memset(server_message, '\0', sizeof(server_message));
						strcpy(server_message, "Hello ");

						/* not error / not full / add all info to the structure*/
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

						strcat(server_message, client_message);
						strcat(server_message, " to the server");

						send(	new_client_socket,
								server_message,
								sizeof(server_message),
								0);

						/* add to the last position in fd_set */
						FD_SET(new_client_socket, &status);

						for(h = 1; h < MAX_CLIENTS ;h++)
						{
							if(stClients[h].iBusy == 0)
							{
								break;
							}

						}

						memset(stClients[h].chMsgBuff, '\0', sizeof(stClients[h].chMsgBuff));
						memset(stClients[h].chUserName, '\0', sizeof(stClients[h].chUserName));
						stClients[h].iBusy = 1;
						stClients[h].isocketId = new_client_socket;
						stClients[h].stClientAddress = new_client_address;
						stClients[h].Dirty = 0;
						strcpy(stClients[h].chUserName, client_message);

					}

				} /* end of listener */
				else
				{
					for(h = 0; h < MAX_CLIENTS ;h++)
					{
						if(stClients[h].isocketId == currentSocket)
						{
							break;
						}

					}
					/* h has the position in the array */


					/* Empty the buffer */
					memset(client_message, '\0', sizeof(client_message));

					/* receive whatever the client sends */
					new = recv(currentSocket, client_message, sizeof(client_message), MSG_DONTWAIT);

					if(new <= 0)
					{
						/* nothing to read.
						 * Check if there was something in the buffer, send it and clean it */
						/* Another child receives the disconnection signal, do nothing */
					}
					else
					{
						if(strcmp(client_message, "!exit") == 0)
						{
							/* client send the exit cmd */
							printf("Child %d disconnecting...\n", getpid());

							/* remove from fd list */
							FD_CLR(stClients[h].isocketId, &status);

							/* close socket */
							close(stClients[h].isocketId);

							/* remove from array */
							stClients[h].iBusy = 0;

							/* decrement counter */
							currentClients--;


							break;
						}
						else
						{
							stClients[h].Dirty = 1;
							strcpy(stClients[h].chMsgBuff, client_message);
						}
					}
				}
			}/* end FD_ISSET */
			else
			{
				for(h = 0; h < MAX_CLIENTS ;h++)
				{
					if(stClients[h].isocketId == currentSocket)
					{
						break;
					}

				}
				if(stClients[h].Dirty == 1)
				{
					for(k = 1; (k < MAX_CLIENTS) ;k++)
					{
						if((k!=h) && (stClients[k].iBusy == 1))
						{
							send(	stClients[k].isocketId,
									stClients[h].chMsgBuff,
									sizeof(stClients[h].chMsgBuff),
									0);
						}
					}
					stClients[h].Dirty = 0;
					strcpy(stClients[h].chMsgBuff, "\0");

				}


			}
		} /* end for */

	} /* end while */

}/* end main */
