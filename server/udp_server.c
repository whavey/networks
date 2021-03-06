#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#define SERVER_UDP_PORT 2476 
#define MAXLEN 4096 

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

int main(int argc, char **argv)
{
    int sd, port, n;
    socklen_t client_len;
    char buf[MAXLEN];
    struct sockaddr_in 	server, client;
    switch(argc) {
        case 2:
            port = SERVER_UDP_PORT;
            break;
        default:
            fprintf(stderr, "Usage: %s [protocol_type]\n", argv[0]);
            fprintf(stderr, "Protocol Types:\n");
            fprintf(stderr, "Default: None\n");
            fprintf(stderr, "1:Stop-and-Wait; 2: Go Back N\n");
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
    char ack[1];
    int pkt_count=0;

    if (atoi(argv[1]) == 1) {   
        // Stop-and-wait control flow --Oi Lee
        // Utilizes Frame and Packet Struct
        printf("%s\n", "Utilizing Stop-and-Wait protocol");
        Frame frame_send;
        Frame frame_recv;
        int frame_id = 0;
        int recv_size;
        int ack = 1;
        fp = fopen(buf,"rb");

        while(1) {
            client_len = sizeof(client);
            if ((recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {
                fprintf(stderr, "Can't receive datagram\n");
                exit(1);
            }
            fp = fopen(buf,"rb");

            // Check if file opened
            if(fp == NULL) {
                printf("Error in file opening\n");
                exit(1);
            }

            while( !feof(fp) ) {
                if (ack == 1) {
                    // create frame
                    frame_send.type = 1;
                    frame_send.seq_no = frame_id;
                    frame_send.ack = 0;
                    frame_send.retrans_no = 0;
                    fread(frame_send.packet.file_buffer,64,1,fp);

                    printf("[+] Sending Frame: %d\n", frame_send.seq_no);
                    sendto(sd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&client, client_len);
                    frame_id++;
                }
                else
                {
                    frame_send.retrans_no++;
                    printf("[+] Resending Frame: %d\n", frame_send.seq_no);
                    sendto(sd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&client, client_len);
                }

                recv_size =  recvfrom(sd, &frame_recv, sizeof(frame_recv), 0, (struct sockaddr *)&client, &client_len);

                if ( recv_size > 0 && frame_recv.type == 0 && frame_recv.ack == frame_send.seq_no)
                {
                    printf("[+] Ack received. Ack #: %d\n", frame_recv.ack);
                    ack = 1;
                }
                else 
                {
                    printf("[-] Ack not received. Frame #: %d\n", frame_send.seq_no);
                    ack = 0;
                }

            }
        }
    } 
    else if (atoi(argv[1]) == 2) {
        // GO BACK N
        printf("%s\n", "Starting Go Back N");
        struct timespec start, end;
        while (1) {
            client_len = sizeof(client);
            if ((recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {
                fprintf(stderr, "Can't receive datagram\n");
                exit(1);
            }
            pkt_count = 0;
            int currentId = 0;
            int lastAck = -1;
            int recentAck = 0;
            int activeTimer = 0;
            fp = fopen(buf,"rb");
            while (!feof(fp)){
                if (activeTimer == 0) {
                    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
                    activeTimer = 1;
                } else {
                    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
                    uint64_t diff = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
                    if (diff > 500) {
                        printf("#TIMEOUT #RETRANSMIT from packet %d\n", lastAck);
                        pkt_count = lastAck;
                        fseek(fp, (64 * pkt_count), SEEK_SET);
                        activeTimer = 0;
                    }
                }
                int curr = htonl(pkt_count);
                sendto(sd, &curr, sizeof(curr), 0, (struct sockaddr *)&client, client_len);
                fread(file_buffer,64,1,fp);
                printf("#TRANSMIT pkt: %d\n",pkt_count);
                sendto(sd, file_buffer, 64, 0, (struct sockaddr *)&client, client_len);
                recvfrom(sd,&recentAck ,sizeof(recentAck),0,(struct sockaddr *)&client, &client_len);
                int parseAck = ntohl(recentAck);
                if (lastAck + 1 == parseAck) {
                    activeTimer = 0;
                    lastAck++;
                }
                printf("#RX_ACK %d\n\n",ntohl(recentAck));
                pkt_count = pkt_count+1;
            }
        }
    }
    else {
        while (1) {
            printf("%s\n", "1");
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
    }

    fclose(fp);
    close(sd);
    return(0);
}
