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

typedef struct packet
{
    char file_buffer[MAXLEN];
}Packet;

typedef struct frame
{
    int type;     // ack = 0; seq = 1; last message for request = 2
    int seq_no;         //Sending sequence #
    int ack;                // Ack #;
    int retrans_no;     // Retransmission count
    Packet packet;
}Frame;

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
    int     i, j, sd; 
    socklen_t server_len;
    char    *pname, *host, rbuf[MAXLEN], sbuf[MAXLEN];
    struct  hostent         *hp;
    struct  sockaddr_in     server;
    struct  timeval         start, end;
    unsigned long address;	
    if (argc != 5) {
        printf("usage: ./UDP_client server file packet_loss protocol_type\n");
        fprintf(stderr, "Protocol Types:\n");
        fprintf(stderr, "Default: None\n");
        fprintf(stderr, "2: Go Back N\n");
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

    FILE *fp;
    fp = fopen(file,"wb");
    char ack[1] = {1};
    char noack[1] = {0};
    struct timeval tv;
    tv.tv_sec = 7;
    tv.tv_usec = 0;

    setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    int drop = atoi(argv[3]);
    int count = 0;
    if ( atoi(argv[4]) == 1 )
    {
        // Stop-and-wait control flow --Oi Lee
        // Utilizes Frame and Packet Struct
        printf("%s\n", "Utilizing Stop-and-Wait protocol");
        Frame frame_send;
        Frame frame_recv;
        int frame_id = 0;
        int recv_size;

        while(1) {
            recv_size = recvfrom(sd, &frame_recv, sizeof(Frame), 0 , (struct sockaddr *)&server, &server_len);

            if (frame_recv.seq_no == frame_id)
            {
                frame_send.seq_no = 0;
                frame_send.type = 0;
                frame_send.ack = frame_recv.seq_no;
                sendto(sd, &frame_send, sizeof(frame_send), 0, (struct sockaddr *)&server, server_len);

                fwrite(frame_recv.packet.file_buffer, 64, 1, fp);
                frame_id++;
            }
        }

        fclose(fp);
        gettimeofday(&end, NULL); /* end delay measurement */
        close(sd);
        printf("#TRANSFER COMPLETE total packets :%d\n", frame_id);

        return(0);
    }
    else if (atoi(argv[4]) == 2) {
        // GO BACK N
        printf("%s\n", "Starting Go Back N");
        int rxId = 0;
        int expectedId = 0;
        int justSkipped = 1;
        while (1) {
            recvfrom(sd, &rxId, sizeof(rxId), 0 , (struct sockaddr *)&server, &server_len);
            int parseId = ntohl(rxId);
            printf("#RCV packet id: %d\n", ntohl(rxId));
            int n = recvfrom(sd, rbuf, 64, 0 , (struct sockaddr *)&server, &server_len);
            if (n <= 0) break;
            if (parseId == expectedId && (rand() % drop) != 0) {
                printf("#ACK packet %d\n", parseId);
                sendto(sd, &rxId, sizeof(rxId), 0, (struct sockaddr *)&server, server_len);
                fwrite(rbuf,64,1,fp);
                expectedId++;
                justSkipped = 0;
            } else {
                printf("#DROP packet id: %d\n", ntohl(rxId));
                int expectedIdParse = htonl(expectedId);
                justSkipped = 1;
                sendto(sd, &expectedIdParse, sizeof(expectedIdParse), 0, (struct sockaddr *)&server, server_len);
            }
        }
        fclose(fp);
        gettimeofday(&end, NULL); /* end delay measurement */
        close(sd);
        printf("#TRANSFER COMPLETE total packets :%d\n", ntohl(rxId));
        return(0);
    } 
    else {
        while (1) {
            int n = recvfrom(sd, rbuf, 64, 0 , (struct sockaddr *)&server, &server_len);
            if (n <= 0) break;
            if (count % drop != 0) {
                sendto(sd, ack, sizeof(int), 0, (struct sockaddr *)&server, server_len);
            }
            else {
                sendto(sd, noack, sizeof(int), 0, (struct sockaddr *)&server, server_len);
            }
            fwrite(rbuf,64,1,fp);
            count = count + 1;
        }
        fclose(fp);
        gettimeofday(&end, NULL); /* end delay measurement */
        close(sd);
        return(0);
    }
}
