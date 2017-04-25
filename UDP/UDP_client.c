//UDP Echo Client
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
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
	int     i, j, sd, server_len, bytes;
	char    *pname, *host, rbuf[MAXLEN], sbuf[MAXLEN], *file;
	struct  hostent         *hp;
	struct  sockaddr_in     server;
	struct  timeval         start, end;
	unsigned long address;	
	if (argc != 5) {
		printf("usage: ./UDP_client server_hostname file_name protocol_type loss_probability\n");
		exit(1);
	}
	char *prot = argv[3];
	if (!(strcmp(prot,"1"))){
		printf("stop and wait selected\n");
	}
	else if (!(strcmp(prot,"2"))){
		printf("GBN selected\n");
	}
	else {
		printf("protocol must be 1 or 2 (stop and wait of GBN)\n");
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
	//if (send(sd, argv[2], data_size, 0, (struct sockaddr *)&server, server_len) == -1) {
	//	fprintf(stderr, "sendto error\n");
	//	exit(1);
	//}
	write(sd,argv[2],strlen(argv[2])+1);
	printf("bytes sent\n");
	char name[32];
	strcpy(name, "/home/whavey/networks/UDP/result/");
	strcat(name, argv[2]);
	FILE *fp = fopen(name, "w");
	while(1){
		//if ((bytes = recvfrom(sd, rbuf, MAXLEN, 0, (struct sockaddr *)&server, &server_len)) < 0) {
		//	fprintf(stderr, "recvfrom error\n");
		//	exit(1);
		//}
		bytes = recv(sd,rbuf,MAXLEN,0);
		if (bytes <= 0) exit(0);
		fwrite(rbuf,1,bytes,fp);
	}
	gettimeofday(&end, NULL); /* end delay measurement */
	if (strncmp(sbuf, rbuf, data_size) != 0) 
		printf("Data is corrupted\n");
	close(sd);
	return(0);
}
