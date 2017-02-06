#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#define SERVER_PORT 2476 
/* arbitrary, but client and server 
   must agree */
#define BUF_SIZE 4096
/* block transfer size */
int main(int argc, char **argv)
{

    int c, s, bytes;
    char buf[BUF_SIZE];
    /* buffer for incoming file */
    struct hostent *h;
    /* info about server */
    struct sockaddr_in channel;
    /* holds IP address */
    if (argc != 3) fatal("Usage: client server-name file-name");
    h = gethostbyname(argv[1]);
    /* look up host's IP 
       address */
    if (!h) fatal("gethostbyname failed");
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) fatal("socket");
    memset(&channel, 0, sizeof(channel));
    channel.sin_family= AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port= htons(SERVER_PORT);
    c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
    if (c < 0) fatal("connect failed");
    /* Connection is now established. Send file name including 0 byte at end. 
     */
    write(s, argv[2], strlen(argv[2])+1);
    /* Go get the file and write it to standard output.*/
    char name[32];
    strcpy(name, "/home/whavey/receive/");
    strcat(name, argv[2]);
    FILE *fp = fopen(name, "w");
    while (1) {
        bytes = recv(s, buf, BUF_SIZE, 0);
        /* read from socket */
        if (bytes <= 0) exit(0);
        /* check for end of file */
	fwrite(buf,1,bytes,fp);
    }
}
fatal(char *string)
{
    printf("%s\n", string);
    exit(1);
}
