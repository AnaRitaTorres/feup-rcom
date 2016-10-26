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
struct linkLayer linkL;


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
		  alarm(linkL.timeout);
		  flag=0;
		}

		while(numberTries < linkL.numTransmissions){

				//sends SET to receiver
				res = write(fd, SET, sizeof(SET));

				if (res == sizeof(SET)){
					break;
				}
		}
		printf("Enviou SET.\n");

		while(numberTries < linkL.numTransmissions){
				success=1;
				//checks if it receives UA
				res = read(fd, buf, sizeof(UA_TRANSMITTER));

				if (res != sizeof(UA_TRANSMITTER)){
					continue;
				}

				for (i=0; i < sizeof(UA_TRANSMITTER); i++){
						if (buf[i] != UA_TRANSMITTER[i]){
							success =0;
							break;
						}
				}
				
				if (success==1){
					if (DEBUG)
						printf("Recebeu UA_TRANSMITTER\n");
					return 0;
					
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
				alarm(linkL.timeout);
			  	flag=0;
			}

			while(numberTries < linkL.numTransmissions){

				//sends SET to receiver
				res = write(fd, DISC_TRANSMITTER, sizeof(DISC_TRANSMITTER));

				if (res == sizeof(DISC_TRANSMITTER)){
					break;
				}
			}
			if (DEBUG)
				printf("Escreveu DISC_TRANSMITTER.\n");

			while(numberTries < linkL.numTransmissions){
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
					if (DEBUG)
						printf("Recebeu DISC_RECEIVER.\n");
					break;
				}
			}

			while(numberTries < linkL.numTransmissions){
				
				res = write(fd, UA_RECEIVER, sizeof(UA_RECEIVER));

				if (res == sizeof(UA_RECEIVER)){
					printf("Enviou UA_RECEIVER.\n");
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
			if (DEBUG)
				printf("Recebeu DISC_TRANSMITTER.\n");
			break;
		}
	}

	while(1){
		res = write(fd, DISC_RECEIVER, sizeof(DISC_RECEIVER));

		if (res== sizeof(DISC_RECEIVER)){
			printf("Enviou DISC_RECEIVER.\n");
			break;
		}
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
			printf("Recebeu UA_RECEIVER.\n");
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
			printf("Recebeu SET.\n");
			break;
		}
	}

	while(1){
		res = write(fd, UA_TRANSMITTER, sizeof(UA_TRANSMITTER));

		if (res == sizeof(UA_TRANSMITTER)){
			printf("Escreveu UA_TRANSMITTER.\n");
			return 0;
		}
	}
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

int numESC(char * buffer, int length){

	int c_ESC=0,i;

	for(i=0; i<length;i++){
		if(buffer[i]==ESC || buffer[i]==FLAG)
			c_ESC++;
	}
	return c_ESC;
}

void stuffing(char *c,char *buf, int length){

    int i,j=0;
	
   
    for(i=0; i < length; i++){

        if(c[i]==FLAG){
          buf[j]=ESC;
		  j++;
          buf[j]=FLAG^STUFF;
        }
        else if(c[i]==ESC){
          buf[j]=ESC;
		  j++;
          buf[j]=ESC^STUFF;
        }
        else
          buf[j]=c[i];

        j++;

    }
    buf[j]='\0';
}


int buildFrame(char * stuffed_buffer, int length){

	//without stuffing
	int i, bcc2;
	
	linkL.frame[0] = FLAG;
	linkL.frame[1] = A_TRANSMITTER;
	linkL.frame[2] = (linkL.sequenceNumber << 6);	// C = 0 S 0 0 0 0 0 0
	linkL.frame[3] = linkL.frame[1]^linkL.frame[2];  //ver se é necessário cópia para guardar dados
	
	for (i=0; i < length; i++){
		linkL.frame[i+4] = stuffed_buffer[i];

		if(i==0)
			bcc2 = stuffed_buffer[i];
		else
			bcc2 ^= stuffed_buffer[i];
	}

	linkL.frame[i+4] = bcc2;
	linkL.frame[i+5] = FLAG;

	return 0;

}


int ll_write(int fd, char * buffer, int length){

	char * stuffed_buf;
	int header_size=4; //F A C BCC1 
	int trailer_size=2; //BCC2 F
	int number_to_stuff = numESC(buffer,length);
	int total_buffer_size = length + number_to_stuff;
	int total_frame_size = length+number_to_stuff+ header_size+ trailer_size;

	
	stuffed_buf = (char*)malloc(length+1+number_to_stuff);

	stuffing(buffer,stuffed_buf,length+1);

	buildFrame(stuffed_buf,total_buffer_size);

}

/*int ll_read(int fd, char * buffer);
*/



int main(int argc, char** argv){

	int c, res;
	char buf[255];
	int i, sum = 0, speed = 0;

	//inicializacoes
	linkL.portName = argv[1];
	linkL.timeout = WAIT_TIME;
	linkL.numTransmissions = NR_TRIES;


	(void) signal(SIGALRM,alarm_handler);

	ll_open(linkL.portName ,TRANSMITTER);

	sleep(2);

	buf[0] = FLAG;
	buf[1] = '1';
	buf[2] = '2';

	ll_write(fd, buf, 3);

	ll_close(fd, TRANSMITTER);

  return 0;
}
