#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define MAX_RECV_LEN      1024
#define PUBLIC_PORT       9000
#define BOARDCAST_IP_ADDR "255.255.255.255"

int socket_fd;
struct sockaddr_in skt_server;
struct sockaddr_in skt_device;
int quit = 0;


void *thread_recv()
{
    char recv[MAX_RECV_LEN] = {0};
    char device_ip[INET_ADDRSTRLEN] = {0};
    int  socket_len         = 0;
    int  ret                = 0;

    socket_len = sizeof(skt_device);

    while(quit != 1) {
        ret = recvfrom(socket_fd, recv, MAX_RECV_LEN, 0, (struct sockaddr *)&skt_device, &socket_len);
        if (ret == -1) {
            perror("can't find any device, wait and try again.\n");
        } else {
            if (ret >= 4) {
               if (recv[0] == 0x0A && recv[1] == 0x01 && recv[2] == 0x02) {
                   inet_ntop(AF_INET, &skt_device.sin_addr, device_ip, sizeof(device_ip));
                   printf("device %s is online.\n", device_ip);
               }
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    char        scan[4]            = {0x0A, 0x01, 0x01, 0x00};
    int         ret                = 0;
    char        device_ip[INET_ADDRSTRLEN] = {0};
    int         socket_len         = 0;
    int         socket_on          = 1;
    int         socket_br          = 1;
    pthread_t   thread_recv_fd;
    
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
    setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &socket_br, sizeof(socket_br));

    skt_server.sin_family = AF_INET;
    skt_server.sin_port = htons(PUBLIC_PORT);
    skt_server.sin_addr.s_addr = inet_addr("BOARDCAST_IP_ADDR");

    socket_len = sizeof(skt_device);

    ret = pthread_create(&thread_recv_fd, NULL, thread_recv, NULL);
    if (ret != 0) {
    	perror("thread create failed!");
	goto EXIT;
    }

    while(quit != 1)
    {
        sendto(socket_fd, scan, 4, 0, (struct sockaddr *)&skt_server, sizeof(skt_server));
        sleep(1);
    }

EXIT:
    pthread_join(thread_recv_fd, NULL);
    close(socket_fd);
    return 0;
}
