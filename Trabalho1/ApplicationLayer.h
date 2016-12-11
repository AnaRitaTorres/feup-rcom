#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <dataLinkLayer.h>
#include <unistd.h>
#include <stdio.h>



#define INPUT_MAX_SIZE 256
#define OSIZE 4


struct packet
{
	char *frame;
	int size;
	
}

