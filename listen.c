#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PUBLIC_PORT       9000

int main(int argc, char **argv)
{
    int                sock_fd;
    char               rcv_buff[1024];
    struct sockaddr_in server_addr;
    struct sockaddr_in device_addr;
    int                socket_len   = 0;
    int                rcv_num      = -1;
    char               server_ip[INET_ADDRSTRLEN] = {0};

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket create error\n");
        exit(1);
    }

    bzero(&device_addr, sizeof(device_addr));
    
    device_addr.sin_family = AF_INET;
    device_addr.sin_port = htons(PUBLIC_PORT);
    device_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socket_len = sizeof(struct sockaddr_in);

    if (bind(sock_fd, (struct sockaddr *)&device_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind error.\n");
        exit(1);
    }
    
    while (1)
    {
	printf("start listening...\n");
        rcv_num = recvfrom(sock_fd, rcv_buff, sizeof(rcv_buff), 0, (struct sockaddr *)&server_addr, &socket_len);
        if (rcv_num > 0) {
	    printf("received num: %d\n", rcv_num);
            inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, sizeof(server_ip));
            if (rcv_num >= 4) {
	        /* HEAD(3 Bytes): 0x0A 0x01 0x01 -- last byte 0x01 means find which device is online.
	           LEN (1 Bytes): len
                */
	        if (rcv_buff[0] == 0x0A && rcv_buff[1] == 0x01 && rcv_buff[2] == 0x01) {
                    printf("server %s is scanning...\n", server_ip);
                    /* online and response my IP address */
                    memset(rcv_buff, 0, sizeof(rcv_buff));
                    /* HEAD(3 Bytes): 0x0A 0x01 0x02 -- last byte 0x02 means this device is online.
                       LEN (1 Bytes): len            -- maybe carries useful data
                    */
                    rcv_buff[0] = 0x0A;
                    rcv_buff[1] = 0x01;
                    rcv_buff[2] = 0x02;
                    rcv_buff[3] = 0x00;
                    rcv_num = sendto(sock_fd, rcv_buff, 4, 0, (struct sockaddr *)&server_addr, socket_len);
                    if (rcv_num == -1) {
                        perror("fedback error!\n");
                    }

                    memset(rcv_buff, 0, sizeof(rcv_buff));
	         }
            } 
            
        }
        else
        {
            perror("recv error\n");
            break;
        }
    }

    close(sock_fd);
    return 0;
}
