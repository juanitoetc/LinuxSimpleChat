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

int main()
{
	char server_message[256] = "Hello from the server";
	int server_socket, client_socket;
	struct sockaddr_in server_address;


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

	client_socket = accept(	server_socket,
							NULL,
							NULL);

	// send the data to the client
	send(	client_socket,
			server_message,
			sizeof(server_message),
			0);

	close(server_socket);

	return 0;

}


