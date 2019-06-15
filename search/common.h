/* Copyright © 2019 Devin. All rights reserved.  
** author: Devin Qian
** email : qg0624@163.com
** date  : 16/06/2019
** description: create this file
*/
#ifndef __COMMON__H__
#define __COMMON__H__

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


#define PUBLIC_PORT         9000
#define BOARDCAST_IP_ADDR   "255.255.255.255"
#define DEVICE_ON_LINE      "Device is online"
#define DEVICE_OFF_LINE	    "Device is offline"

typedef enum {
    DEVICE_TYPE_LAMP,
    DEVICE_TYPE_CAMERA,
} device_type;

typedef enum {
    DEVICE_OFFLINE,
    DEVICE_ONLINE   
} device_status;

typedef struct {
    device_type     dev_type;
    unsigned int    dev_id;
    unsigned char   dev_name[32];
    device_status   dev_status;
    char            dev_ip[INET_ADDRSTRLEN];
} device_info;


#endif
