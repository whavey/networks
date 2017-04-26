#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define SERVER_UDP_PORT 2476 
#define MAXLEN 4096 

int main(int argc, char **argv)
{
	int sd, client_len, port, n;
	char buf[MAXLEN];
	struct sockaddr_in 	server, client;
	switch(argc) {
		case 1:
			port = SERVER_UDP_PORT;
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}
	/* Create a datagram socket */
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}
	/* Bind an address to the socket */
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}
	FILE *fp;
	size_t file_size;
	char file_buffer[2000];
	int bytes;
	char ack[1];
	int pkt_count=0;
	while (1) {
		client_len = sizeof(client);
		if ((recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {
			fprintf(stderr, "Can't receive datagram\n");
			exit(1);
		}
		fp = fopen(buf,"rb");
		while (!feof(fp)){
			fread(file_buffer,64,1,fp);
			printf("sending pkt: %d\n",pkt_count);
			sendto(sd, file_buffer, 64, 0, (struct sockaddr *)&client, client_len);
			recvfrom(sd,ack,1,0,(struct sockaddr *)&client, &client_len);
			printf("%d ack: %d\n\n",pkt_count,ack[0]);
			pkt_count = pkt_count+1;
		}
	}
	fclose(fp);
	close(sd);
	return(0);
}
