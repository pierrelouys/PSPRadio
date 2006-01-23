#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#define BUFSIZE		1024

void Die(char *message)
{
	perror(message);
	exit(1);
}

int main (int argc, char *argv[])
{
	int				sock;
	struct			sockaddr_in	log_server;
	struct			sockaddr_in	log_client;
	char			buffer[BUFSIZE];
	unsigned int	client_len, server_len;
	int				received = 0;

	fprintf(stdout, "LogTool v0.1\n\n");


	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	/* Create UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		{
		Die("Failed to create socket..");
		}

	/* Create sockaddr structure */
	memset(&log_server, 0, sizeof(log_server));
	log_server.sin_family		= AF_INET;
	log_server.sin_addr.s_addr	= htonl(INADDR_ANY);
	log_server.sin_port 		= htons(atoi(argv[1]));

	/* Bind the socket */
	server_len = sizeof(log_server);
	if (bind(sock, (struct sockaddr *) &log_server, server_len) < 0)
	{
		Die("Failed to bind server socket");
		
	}

	/* Run until cancelled */
	while (1)
	{
		/* Receive datagram from client */
		client_len = sizeof(log_client);
		if ((received = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *) &log_client, &client_len)) < 0)
		{
			Die("Failed to receive message from client..");
		}
		buffer[received] = 0x00;
		fprintf(stdout, "> %s\n", buffer);
	}

	exit(0);
}
