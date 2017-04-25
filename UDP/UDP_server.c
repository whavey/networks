/* Echo server using UDP */
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define SERVER_UDP_PORT 2476 
#define MAXLEN 4096
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	int sd, client_len, port, n, fd;
	char buf[MAXLEN];
	char *prot;
	struct sockaddr_in server, client;
	if (argc != 3) {
		printf("usage: ./UDP_server loss_probability protocol\n");
		exit(1);
	}
	if (!(strcmp(argv[2], "1"))){
		printf("Stop and wait selected\n");
	}
	else if(!(strcmp(argv[2], "2"))){
		printf("GBN selected\n");
	}
	else{
		printf("protocol must be 1 or 2 (stop and wait or GBN)\n");
		exit(1);
	}
	/* Create a datagram socket */
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}
	/* Bind an address to the socket */
	bzero((char *)&server, sizeof(server));
	server.sin_family= AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}
	int bytes;
	while (1) {
		printf("in first while\n");
		client_len = sizeof(client);
		//if ((n = recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {
		//	fprintf(stderr, "Can't receive datagram\n");
		//	exit(1);
		//}
		read(sd,buf,MAXLEN);
		printf("received data\n");
		fd = open(buf, O_RDONLY);
		while(1) {
			printf("in second while\n");
			bytes = read(fd, buf, MAXLEN);
			printf("bytes:%d\n",bytes);
			if (bytes < 0) break;
			if (sendto(sd, buf, n, 0, (struct sockaddr *)&client, client_len) != n) {
				fprintf(stderr, "Can't send datagram\n");
				exit(1);
			}
		}

	}
	prinf("Left both whiles\n");
	close(fd);
	close(sd);
	return(0);
}
