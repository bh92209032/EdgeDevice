#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <jansson.h>
#include <curl/curl.h>
#include <ctype.h>
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
	char lat[16];
	char latNS;
	char lon[16];
	char lonEW;
	char mode;

} GPGGA_Struct; 

struct string
{
        char *ptr;
        size_t len;
};

void init_string(struct string *s)
{
	s->len = 0;
	s->ptr = malloc(s->len + 1);

	if(s->ptr == NULL) 
	{
		fprintf(stderr," malloc() fail\n");
		exit(EXIT_FAILURE);
	}

	s->ptr[0] = 0;
}

static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size * nmemb;

	s->ptr = realloc(s->ptr, new_len + 1);

	if(s->ptr == NULL) 
	{
		fprintf(stderr, "ralloc fail\n");
		exit(EXIT_FAILURE);
	}

	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = 0;
	s->len = new_len;

	return size*nmemb;
}

int Send_GPSData(char *url, char *data)
{
	CURL *curl;
	CURLcode res;
	struct string repost;

	printf("%s, [%s]%s\r\n", __func__, url, data);

	init_string(&repost);

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_DEFAULT); // 이 옵션은 thread 메모리 공유에 안전하지 않다. 나는 주석처리함

	/* get a curl handle */
	curl = curl_easy_init();

	struct curl_slist *list = NULL;

	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, url); //webserver ip 주소와 포트번호, flask 대상 router
#if 1

		list = curl_slist_append(list, "Content-Type: application/json"); // content-type 정의 내용 list에 저장 
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list); // content-type 설정
#if 0
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 값을 false 하면 에러가 떠서 공식 문서 참고함 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // 값을 false 하면 에러가 떠서 공식 문서 참고함
#endif
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300);

		curl_easy_setopt(curl, CURLOPT_POST, 1L); //POST option
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); //string의 data라는 내용을 전송 할것이다
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); //string의 data라는 내용을 전송 할것이다
#if 0
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //string의 data라는 내용을 전송 할것이다
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &repost); //string의 data라는 내용을 전송 할것이다
#endif
#endif
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl); // curl 실행 res는 curl 실행후 응답내용이 
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl); // curl_easy_init 과 세트
		curl_global_cleanup(); // curl_global_init 과 세트
		curl_slist_free_all(list); // CURLOPT_HTTPHEADER 와 세트

		printf("====================================\r\n");
		printf(" CURL Send Data : \r\n");
		printf(" \t----> %s\r\n", data);
		printf("====================================\r\n");


		return 1;
	}
	return 0;
}

int Create_JsonMessage(float temp, char *msg_dmp) 
{
        json_t* root = json_object();
        json_t* data = json_object();
        json_object_set_new(root, "indexName", json_string("temp_index"));
        json_object_set_new(root, "deviceId", json_string("Sensor02"));
        json_object_set_new(root, "devicename", json_string("temp_sensor"));

        json_object_set_new(data, "temp", json_real(temp));
        json_object_set(root, "data", data);

        msg_dmp = json_dumps(root, JSON_PRESERVE_ORDER);

	printf("%s\n", msg_dmp);

        json_decref(root);
        return msg_dmp;
}
	
void Data_Parser(uint8_t *buff, int len)
{
	int i, j, k;
	char msg[128], temp[8];
	char *p;
	float f;
	
	printf("%s, %s\r\n", __func__, buff);

	if(p = strstr(buff, "TEMP : ")) 
	{
		p += strlen("TEMP : "); 
		printf("%s, %s\r\n", __func__, p);
		{
			memset(temp, 0, sizeof(temp));
				
			for(i=0, j=0, k=0; i<(len - strlen("DATA : TEMP : ")); i++)
			{ 
				if( *(p+i) == ',') 
				{
					break;
				}
				else
				{
					temp[i] = *(p+i); 
				}
			}
	

			f = atof(temp);

			printf("\r\n================================\r\n");
			printf("\t%s\r\n", __func__);
			printf("================================\r\n");
			printf(" 1. RawMesg [%s]\r\n", buff);	
			printf(" 2. TEMP  : %s\r\n", temp);	
			printf("================================\r\n");

			memset(msg, 0, sizeof(msg));

			Send_GPSData("http://enitt.iptime.org:8080/temp", Create_JsonMessage(f, &msg)); 
		}
	}
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
#if 1
                timeout.tv_sec = 5;
                timeout.tv_usec = 0; // 1ms
                temps = reads;

                rc = select(fd+1, &temps, NULL, NULL, &timeout );

                if(rc == -1)
                {
                        printf("[MCA ETH_UI] select error\n");
                        return -1;
                }

                if ( !FD_ISSET(fd, &temps ) )        
		{
			memset(&buff, 0, fd_index); 
			fd_index = 0; 
			msgend = 0;
			continue;
		}

                n_read = read(fd, msg, sizeof(msg));
#if 0
                for(i=0; i<n_read; i++)
                {
                        //if(msg[i] != '\n' && msg[i] != '\r')
                                printf("%c",msg[i]);
                }
		printf("\n\r");
#else
		msgend = 0; 

                for(i=0; i<n_read; i++)
                {
                        if(msg[i] == '\n' || msg[i] == '\r') 
			{
				msgend = 1; 
			}
			else buff[fd_index++] = msg[i]; 
		}
			
		if(msgend == 1)
		{	
			Data_Parser(buff, fd_index); 
			memset(&buff, 0, fd_index); 
			fd_index = 0; 
			msgend = 0;
		}
#endif
#else
			float temp = 36.86;
	
			printf("\r\n================================\r\n");
			printf("\t%s\r\n", __func__);
			printf("================================\r\n");
			printf(" 1. RawMesg [%s]\r\n", buff);	
			printf(" 2. TEMP  : %.3f\r\n", temp);	
			printf("================================\r\n");

			memset(msg, 0, sizeof(msg));

			Send_GPSData("http://enitt.iptime.org:8080/temp", Create_JsonMessage(temp, &msg)); 
	
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

        ttyfd = tty_open(argv[1], argv[2]);

	if(ttyfd > 0)
	{
		printf("%s > Open Success\n\r", __func__); 
	}
	else
	{
		printf("%s > Open Fail\n\r", __func__); 
		return 0;
	}

	memset(msg, 0, sizeof(msg));

	Chelsea_Communication_RxTask(ttyfd); 
}

