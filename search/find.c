/* Copyright © 2019 Devin. All rights reserved.  
** author: Devin Qian
** email : qg0624@163.com
** date  : 1/1/2019
** description: create this file
*/

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
#include "common.h"

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
    int  socket_len         = 0;
    int  ret                = 0;
    device_info dev_info    = {0};
    int  len                = 0;

    socket_len = sizeof(skt_device);

    while(quit != 1) {
        ret = recvfrom(socket_fd, recv, MAX_RECV_LEN, 0, (struct sockaddr *)&skt_device, &socket_len);
        if (ret == -1) {
            perror("can't find any device, wait and try again.\n");
        } else {
            if (ret >= 6) {
               if (recv[0] == 0x0A && recv[1] == 0x02) {
                   memset(&dev_info, 0, sizeof(dev_info));
                   inet_ntop(AF_INET, &skt_device.sin_addr, dev_info.dev_ip, sizeof(dev_info.dev_ip));
                   
                   dev_info.dev_type   = recv[2];
                   dev_info.dev_id     = recv[3];
                   dev_info.dev_status = (recv[4] == DEVICE_ONLINE) ? DEVICE_ONLINE : DEVICE_OFFLINE;
                   
                   len = (recv[5] >= 32) ? 32 : recv[5];
                   
                   memcpy(&dev_info.dev_name, &recv[6], len);
                   
                   printf("device(%s:%d): %d", dev_info.dev_name, dev_info.dev_type, dev_info.dev_id);
                   printf("ip:%s is %s.\n", dev_info.dev_ip, (dev_info.dev_status == DEVICE_ONLINE) ? "ONLINE" : "OFFLINE");
               }
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    char        scan[4]            = {0x0A, 0x01, 0x00, 0x00};
    int         ret                = 0;
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
        sleep(5);
    }

EXIT:
    pthread_join(thread_recv_fd, NULL);
    close(socket_fd);
    return 0;
}
