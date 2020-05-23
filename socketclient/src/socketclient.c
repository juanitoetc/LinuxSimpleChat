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

int main()
{
	int network_socket, connection_status;
	struct sockaddr_in server_address;

	char server_response[256];

	network_socket = socket(AF_INET,
							SOCK_STREAM,
							0);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	connection_status = connect(	network_socket,
									(struct sockadd *) &server_address,
									sizeof(server_address));


	if(connection_status == -1)
	{
		printf("error connection to the remote socket\n");
		close(network_socket);
		return 0;
	}

	recv(	network_socket,
			&server_response,
			sizeof(server_response),
			0);

	// print the data from the server
	printf("Server data: %s\n", server_response);

	close(network_socket);

	return 0;
}