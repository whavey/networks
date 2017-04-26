//UDP Echo Client
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#define SERVER_UDP_PORT         2476
#define MAXLEN                  4096
#define DEFLEN                  64 
long delay(struct timeval t1, struct timeval t2)
{
	long d;
	d = (t2.tv_sec -t1.tv_sec) * 1000;
	d += ((t2.tv_usec -t1.tv_usec + 500) / 1000);
	return(d);
}
int main(int argc, char **argv)
{
	int     data_size = DEFLEN, port = SERVER_UDP_PORT;
	int     i, j, sd, server_len;
	char    *pname, *host, rbuf[MAXLEN], sbuf[MAXLEN];
	struct  hostent         *hp;
	struct  sockaddr_in     server;
	struct  timeval         start, end;
	unsigned long address;	
	if (argc != 5) {
		printf("usage: ./UDP_client server file packet_loss protocol_type\n");
		exit(1);
	}
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "Can't get server's IP address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);
	if (data_size > MAXLEN) {
		fprintf(stderr, "Data is too big\n");
		exit(1);
	}
	gettimeofday(&start, NULL); /* start delay measurement */
	server_len = sizeof(server);
	char *file = argv[2];
	if (sendto(sd, file, data_size, 0, (struct sockaddr *)&server, server_len) == -1) {
		fprintf(stderr, "sendto error\n");
		exit(1);
	}
	
	FILE *fp = fopen("/home/whavey/networks/client/receive","wb"); 
	char ack[1] = {1};
	char noack[1] = {0};
	struct timeval tv;
	tv.tv_sec = 7;
	tv.tv_usec = 0;

	setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

	recvfrom(sd, rbuf, 64, 0, (struct sockaddr *)&server, &server_len);
	sendto(sd, ack, sizeof(int), 0, (struct sockaddr *)&server, server_len);
	int drop = atoi(argv[3]);
	int count = 0;
	while (1) { 
		fwrite(rbuf,64,1,fp);
		int n = recvfrom(sd, rbuf, 64, 0 , (struct sockaddr *)&server, &server_len);
		if (n <= 0) break;
		if (count % drop != 0) {
			sendto(sd, ack, sizeof(int), 0, (struct sockaddr *)&server, server_len);
		}
		else {
			sendto(sd, noack, sizeof(int), 0, (struct sockaddr *)&server, server_len);
		}
		count = count + 1;

	}
	fclose(fp);
	gettimeofday(&end, NULL); /* end delay measurement */
	close(sd);
	return(0);
}
