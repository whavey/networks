#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netinet/in.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
	if (argc != 5) {
		printf("usage: ./client server_name file loss_probability protocol\n");
		exit(1);
	}
	if (!(strcmp(argv[4],"1"))) {
		printf("s&w selected\n");
	}
	else if (!(strcmp(argv[4],"2"))) {
		printf("GBN selected\n");
	}
	else {
		printf("protocol must be 1 or 2 (s&w or GBN)\n");
		exit(1);
	}
	char buff[2000];
	int sockfd,connfd,len;

	struct sockaddr_in servaddr,cliaddr;

	//create socket in client side
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sockfd==-1)
	{
		printf(" socket not created in client\n");
		exit(0);
	}
	else
	{
		printf("socket created in  client\n");
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY; // ANY address or use specific address
	servaddr.sin_port = htons(2476);  // Port address

	// send  msg to server

	sendto(sockfd, argv[2], strlen(buff), 0,
			(struct sockaddr *)&servaddr, sizeof(struct sockaddr));
	char file_buffer[2000];

	if (recvfrom(sockfd,file_buffer,2000,0,  (struct sockaddr *)&servaddr, sizeof(struct sockaddr))<0)
	{
		printf("error in recieving the file\n");
		exit(1);
	}
	printf("received");
	char new_file[]="";
	strcat(new_file,buff);
	FILE *fp;
	fp=fopen(new_file,"w+");
	if(fwrite(file_buffer,1,sizeof(file_buffer),fp)<0)
	{
		printf("error writting file\n");
		exit(1);
	}


	//close client side connection
	close(sockfd);

	return(0);
}
