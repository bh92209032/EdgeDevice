#include "common.h"

int main(int argc, char **argv)
{
	int i;
	char msg[128];
	int len = 0;

	memset(msg, 0, sizeof(msg));

	sprintf(msg, "at+qcfg=\"%s\",%x,%x,%x,%x\r\n", "band", 0x93,0x80094,1,1);

	len = strlen(msg);

	//msg[len++] = 0x0d;
	//msg[len++] = 0x0a;

	for(i=0; i<len; i++)
	{
		printf("%02x ", msg[i]);
	}
	printf("\n\r");


	return 0;

}

