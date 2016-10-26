#include "dataLinkLayer.h"

/* Global and Const variables */
const unsigned char SET[] = {FLAG, A_TRANSMITTER, C_SET, A_TRANSMITTER^C_SET, FLAG};
const unsigned char UA_TRANSMITTER[] = {FLAG, A_TRANSMITTER, C_UA, A_TRANSMITTER^C_UA, FLAG};
const unsigned char UA_RECEIVER[] = {FLAG, A_RECEIVER, C_UA,  A_RECEIVER^C_UA, FLAG};
const unsigned char DISC_TRANSMITTER[] = {FLAG, A_TRANSMITTER, C_DISC, A_RECEIVER^C_DISC, FLAG};
const unsigned char DISC_RECEIVER[] = {FLAG, A_RECEIVER, C_DISC, A_TRANSMITTER^C_DISC, FLAG};

int filedes, numberTries = 0, fd;
int flag=1;

struct termios oldtio,newtio;


void alarm_handler(){

	printf("alarme # %d\n", numberTries);
	flag=1;
	numberTries++;

}

int sendSET(int fd){

		int res, success, i;
		char buf[255];

		//enables the alarm WAIT_TIME seconds
		if(flag){
		  alarm(WAIT_TIME);
		  flag=0;
		}

		while(numberTries < NR_TRIES){

				//sends SET to receiver
				res = write(fd, SET, sizeof(SET));

				if (res == sizeof(SET)){
					break;
				}
		}
		printf("Enviou SET.\n");

		while(numberTries < NR_TRIES){
				success=1;
				//checks if it receives UA
				res = read(fd, buf, sizeof(UA_TRANSMITTER));

				if (res != sizeof(UA_TRANSMITTER)){
					continue;
				}

				for (i=0; i<sizeof(UA_TRANSMITTER); i++){
						if (buf[i] != UA_TRANSMITTER[i]){
							success =0;
							break;
						}
				}
				
				if (success==1){
					printf("\nUA_TRANSMITTER: ");
					for (i=0; i<sizeof(UA_TRANSMITTER); i++)
						printf(" %02x ", buf[i]);
					return 0;
					
				}
				if (DEBUG){
					printf("Informação errada.\n");
				}
		}

		return -1;
	}

	int sendDISC(int fd){
			int res, success, i;
			char buf[255];
			numberTries=0;

			//enables the alarm WAIT_TIME seconds
			if(flag){
				alarm(WAIT_TIME);
			  	flag=0;
			}

			while(numberTries < NR_TRIES){

				//sends SET to receiver
				res = write(fd, DISC_TRANSMITTER, sizeof(DISC_TRANSMITTER));

				if (res == sizeof(DISC_TRANSMITTER)){
					break;
				}
			}

			printf("\nEscreveu DISC_TRANSMITTER");

			while(numberTries<NR_TRIES){
				success=1;
				//checks if it receives anything
				res = read(fd, buf, sizeof(DISC_RECEIVER));

				if (res != sizeof(DISC_RECEIVER)){
						continue;
				}

				for (i=0; i<sizeof(DISC_RECEIVER); i++){
					if (buf[i] != DISC_RECEIVER[i]){
						success =0;
						break;
					}
				}

				if (success == 1){
					printf("\nDISC_RECEIVER: ");
					for(i=0; i<sizeof(DISC_RECEIVER); i++){
						printf(" %02x ", buf[i]);
					}
					break;
				}
			}

			while(numberTries<NR_TRIES){
				
				res = write(fd, UA_RECEIVER, sizeof(UA_RECEIVER));

				if (DEBUG){
					printf("Enviados %d bytes do UA_RECEIVER.\n", res);
				}

				if (res == sizeof(UA_RECEIVER)){
					return 0;
				}
			}
	return -1;
}


int receiveDISC(int fd){
	int res, i, success;
	char buf[255];
	while(1){
		success=1;
		res =read(fd, buf,sizeof(DISC_TRANSMITTER));

	   	if(res != sizeof(DISC_TRANSMITTER))
		   continue;

		for(i=0; i < sizeof(DISC_TRANSMITTER); i++){

			if(buf[i]!= DISC_TRANSMITTER[i]){
				success = 0;
				break;
			}
		}

		if (success = 1){
			printf("disc transmitter: ");
			for (i=0; i<sizeof(DISC_TRANSMITTER); i++){
				printf(" %02x ", buf[i]);
			}
			break;
		}
	}

	while(1){
		res = write(fd,DISC_RECEIVER,sizeof(DISC_RECEIVER));

		if (res== sizeof(DISC_RECEIVER))
			break;
	}

	while(1){
		success = 1;

		res = read(fd, buf,sizeof(UA_RECEIVER));

		if(res != sizeof(UA_RECEIVER))
			continue;

		for(i=0; i < sizeof(UA_RECEIVER);i++){

			if(buf[i]!= UA_RECEIVER[i]){
				success = 0;
			    break;
			}
		}

		if (success){
			printf("\nUA_RECEIVER:\n");
			for(i=0; i < sizeof(UA_RECEIVER);i++)
				printf(" %02x ", buf[i]);
			return 0;
		}
	}

	return -1;
}

int receiveSET(int fd){

	int res, i, success;
	char buf[255];
	while(1){
		success=1;
		res =read(fd, buf,sizeof(SET));

	   	if(res != sizeof(SET))
		   continue;
		
		for(i=0; i < sizeof(SET);i++){

			if(buf[i]!= SET[i]){
				success=0;
			    break;
			}
		}
		if (success==1){
			printf("\nSET: ");
			for(i=0;i<sizeof(SET); i++)
				printf(" %02x ",buf[i]);
			break;
		}
	}

	while(1){
		res = write(fd, UA_TRANSMITTER, sizeof(UA_TRANSMITTER));

		if (res == sizeof(UA_TRANSMITTER)){
			printf("\nEscreveu UA_TRANSMITTER");
			return 0;
		}
	}
}






int stuffing(unsigned char byte){
		if(byte==FLAG){
			//FLAG = 0x7D 0x5E;
			return 0;
		}
		else if (byte==ESCAPE){
			//ESCAPE = 0x7D 0x5D;
			return 0;
		}
		else return -1;
}

int ll_open(char *portName,int status){

	/*opening the port*/
	fd = open(portName, O_RDWR | O_NOCTTY);

	if (fd <0) {
		perror(portName);
		exit(-1);
	}

	/* setting up the port*/
	if ( tcgetattr(fd,&oldtio) == -1) {
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

	tcflush(fd, TCIOFLUSH);


	if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	if(status==TRANSMITTER){
		if(sendSET(fd)==-1){
			return -1;
		}
	}
	else if(status==RECEIVER){
		if(receiveSET(fd)==-1){
			return -1;
		}
	}
	else
			return -1;

	return 0;
}


int ll_close(int fd, int status)
{
		if(status==TRANSMITTER){
			sendDISC(fd);
		}
		else if (status==RECEIVER){
			receiveDISC(fd);
		}
		else
				return -1;



		/*closing the port*/
		if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
			perror("tcsetattr");
			exit(-1);
		}

		tcsetattr(fd,TCSANOW,&oldtio);
		close(fd);
		return 0;
}

/*int llwrite(int fd,char * buffer, int length);

int llread(int fd, char * buffer);
*/



int main(int argc, char** argv){

	int c, res;
	char buf[255];
	int i, sum = 0, speed = 0;
	struct linkLayer linkL;

	(void) signal(SIGALRM,alarm_handler);


	linkL.portName = argv[1];

	ll_open(linkL.portName ,TRANSMITTER);

	sleep(2);


	ll_close(fd, TRANSMITTER);

  return 0;
}
