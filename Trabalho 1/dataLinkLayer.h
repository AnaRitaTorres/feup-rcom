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
#include <time.h>

/* Definitions */
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define ESC 0x7D
#define A_TRANSMITTER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0X0B
#define C_REJ 0x01
#define C_RR 0x05
#define STUFF 0x20

#define NR_TRIES  3
#define WAIT_TIME 3

#define TRANSMITTER 0
#define RECEIVER 1

#define MAX_SIZE 255

#define DEBUG 1

struct linkLayer {
	char * portName;
	int baudRate;
	int status;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
};


typedef enum {START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV} State;

void alarm_handler();

int sendSET(int fd);

int sendDISC(int fd);

int receiveDISC(int fd);

int receiveSET(int fd);

int numESC(char * buffer, int length);

void stuffing(char *to_stuff,char *stuffing, int length);

void desstuffing(char *to_desstuff,char*desstuffing, int length);

int writeInfo(const unsigned char *buffer, int length);

int readInfo(unsigned char *buffer, int length);

int readInfoTimeOut(unsigned char *buffer, int length);

int ll_open(char *portName);

int ll_close(int fd);

int ll_write(int fd,char * buffer, int length);

int ll_read(int fd, char * buffer, int length);
