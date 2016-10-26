#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

/* Definitions */
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define ESCAPE 0x7D
#define A_TRANSMITTER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0X0B

#define NR_TRIES  3
#define WAIT_TIME 3

#define TRANSMITTER 0
#define RECEIVER 1

#define MAX_SIZE 255

#define DEBUG 1

struct linkLayer {
	char * portName;
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
};

void alarm_handler();

int sendSET(int fd);

int sendDISC(int fd);

int receiveSET(int fd);

int stuffing(unsigned char byte);

int ll_open(char *portName,int status);

int ll_close(int fd, int status);

int llwrite(int fd,char * buffer, int length);

int llread(int fd, char * buffer);
