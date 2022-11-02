#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include "common.h"

static struct termio oldtio,newtio;

int  ttyfd;
int n_read = 0;
int rcv_len=0;

int tty_open(char *tty_port, char *baud)
{
	int dev = open(tty_port, O_RDWR);
	int baud_rate = 0;
	
        if (dev < 0)
        {
                printf("[IPC_UI] ipc_if(%s) open fail.\r\n", tty_port);
                exit(-1);
        }

	if(!strcmp("9600", baud)) baud_rate = B9600;
	else if(!strcmp("19200",baud)) baud_rate = B19200;
	else if(!strcmp("38400", baud)) baud_rate = B38400;
	else if(!strcmp("57600", baud)) baud_rate = B57600;
	else if(!strcmp("115200", baud)) baud_rate = B115200;
	else return -1;
	


        /* serial port setting */
        tcgetattr(dev,&oldtio); /* save current serial port settings */
//        bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
	memset(&newtio, 0, sizeof(newtio)); /* clear struct for new port settings */
        newtio.c_cflag = baud_rate | CS8 | CREAD | CLOCAL;
        newtio.c_iflag = IGNPAR; // No Parity
        newtio.c_oflag = 0;
        tcsetattr(dev,TCSANOW,&newtio);

        return dev;
}

int MsgStatusFlag = 0; 

typedef struct
{
	float Axial;
	float Radail;
	float Tangential;
	float Overall;

} RTU_Message; 

void Data_Parser(uint8_t *buff, int len)
{
	RTU_Message *rtu = (RTU_Message *)buff;
	
	printf("\r\n=======================================\r\n");
	printf("\t%s\r\n", __func__);
	printf("=======================================\r\n");
	printf(" Axial : %f\r\n", rtu->Axial); 
	printf(" Radail : %f\r\n", rtu->Radail); 
	printf(" Tangential : %f\r\n", rtu->Tangential); 
	printf(" Overall : %f\r\n", rtu->Overall); 
	printf("=======================================\r\n");
} 

void Chelsea_Communication_RxTask(int  fd) 
{
	int msgend = 0; 
	struct timeval timeout;
	fd_set reads, temps;
	int fd_max, rc;
	int len, write_len = 0;
	int fd_index = 0; 
	char msg[256];
	uint8_t buff[256];
	int i;

	FD_ZERO( &reads );
	FD_SET( fd, &reads );

        while(1)
        {
                timeout.tv_sec = 5;
                timeout.tv_usec = 0; // 1ms
                temps = reads;

                rc = select(fd+1, &temps, NULL, NULL, &timeout );

                if(rc == -1)
                {
                        printf("[MCA ETH_UI] select error\n");
                        return -1;
                }

                if ( !FD_ISSET(fd, &temps ) )        continue;

                n_read = read(fd, msg, sizeof(msg));
#if 0 
                for(i=0; i<n_read; i++)
                {
                        if(msg[i] != '\n' && msg[i] != '\r')
                                printf("%c",msg[i]);
                }
		printf("\n\r");
#else
                for(i=0; i<n_read; i++)
                {
			buff[fd_index++] = msg[i]; 
			if(fd_index >= sizeof(RTU_Message)) msgend = 1; 
		}
			
		if(msgend == 1)
		{	
			Data_Parser(buff, fd_index); 
			memset(&buff, 0, fd_index); 
			fd_index = 0; 
			msgend = 0;
		}
#endif
        }

	printf("%s > EXIT_SUCCESS.\n\r", __func__); 
	close(fd);
	exit(EXIT_SUCCESS);

}

int main(int argc, char **argv)
{
	char msg[256];

	if(argc < 3) 
	{
		printf("%s > Error, Check the argument\r\n", __func__); 

		return 0;
	}

	sprintf(msg, "/dev/%s", argv[1]); 

        ttyfd = tty_open(msg, argv[2]);


	if(ttyfd > 0)
	{
		printf("%s > (%s) Open Success\n\r", __func__, msg); 
	}
	else
	{
		printf("%s > (%s) Open Fail\n\r", __func__, msg); 
		return 0;
	}


        system("echo 114 > /sys/class/gpio/export");    // MODBUS3_UART6_TXEN
        system("echo out > /sys/class/gpio/gpio114/direction");
	system("echo 0 > /sys/class/gpio/gpio114/value");

	Chelsea_Communication_RxTask(ttyfd); 
}

