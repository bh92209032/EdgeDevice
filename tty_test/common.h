#ifndef _HCU_COMMON_H_
#define _HCU_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <sys/time.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <sys/ipc.h>
#include <sys/msg.h>

//##############################################################################
//			RESOURES
//##############################################################################

#define BAUDRATE B115200
#define termio  termios

#define RS232   0	// DEBUG
#define MODBUS_RS485_1 1
#define MODBUS_RS485_2 2
#define MODBUS_RS485_3 3
#define MODBUS_RS485_4 4
#define MODBUS_RS485_5 5
//#define MODBUS_RS485_6 6

#define USB0	
#define USB1
#define USB2
#define USB3

/* IOCTL Define */
#define NODE_FILE "/dev/gpio"
#define RD      0
#define WR      1

#define HIGH    1
#define LOW     0


enum
{
        KU_TXEN=0,
        EXT_TXEN,
};

typedef struct __gpio_info__
{
        unsigned int num;
        unsigned int val;
}GPIO_INFO;
#endif
