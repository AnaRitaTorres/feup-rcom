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


int equal(unsigned char * buffer, const unsigned char * toCompare, int length){
	int success = 0, i;
	for (i=0; i < length; i++){
		if (buffer[i] != toCompare[i]){
			success = -1;
			break;
		}
	}
	return success;
}

int writeInfo(const unsigned char *buffer, int length){

	int res, aux=0;
	while(aux < length){

		res = write(fd, buffer + aux, length - aux);

		if (res == length)
			return 0;

		aux += res;

		if (res == -1)
			return -1;

	}
	return 0;
}

int readInfo(unsigned char *buffer, int length){
	int res;
	int aux=0;

	while (aux < length && numberTries<linkL.numTransmissions){

		res = read(fd, buffer+aux, length-aux);

		aux+=res;

		if (res == -1)
			return -1;
	}
	
	return 0;

}


int readInfoTimeOut(unsigned char *buffer, int length){
	int res;
	int aux=0;

	while (aux < length && numberTries<linkL.numTransmissions){

		if (flag){
			alarm(1);
			flag=0;
		}

		res = read(fd, buffer+aux, length-aux);

		aux+=res;

		if (res == -1)
			return -1;
	}

	if (numberTries == linkL.numTransmissions)
		return -1;

	return 0;

}


int sendSET(int fd){

	unsigned char buf[255];
	
	while(numberTries < linkL.numTransmissions){

		if (writeInfo(SET, (int)sizeof(SET)) == -1){
			continue;
		}
		printf("Escreveu SET.\n");

		if (readInfoTimeOut(buf, (int)sizeof(UA_TRANSMITTER)) == -1){
			numberTries=0; //acabar se não der ou reenviar??
			continue;
		}

		if (equal(buf, UA_TRANSMITTER, (int)sizeof(UA_TRANSMITTER)) == 0){
			printf("Recebeu UA.\n");
			return 0;
		}
	}

	return -1;
}

int receiveSET(int fd){

	char buf[255];

	while(1){

		if (readInfo(buf, (int)sizeof(SET)) == -1){
			printf("Não leu.\n");
			continue;
		}
		
		if (equal(buf, SET, (int)sizeof(SET)) == -1){
			printf("Não recebeu SET.\n");
			continue;
		}

		if (writeInfo(UA_TRANSMITTER, (int)sizeof(UA_TRANSMITTER)) == -1){
			printf("Não escreveu.\n");
			continue;
		}

		printf("Escreveu UA.\n");
		return 0;
	}
}

int ll_open(char *portName){

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

	if(linkL.status==TRANSMITTER){
		if(sendSET(fd)==-1){
			return -1;
		}
	}
	else if(linkL.status==RECEIVER){
		if(receiveSET(fd)==-1){
			return -1;
		}
	}
	else
		return -1;

	return 0;
}

int sendDISC(int fd){
	
	char buf[255];
	numberTries=0;

	while(numberTries < linkL.numTransmissions){

		if (writeInfo(DISC_TRANSMITTER, (int)sizeof(DISC_TRANSMITTER)) == -1){
			continue;
		}

		printf("Escreveu Disc.\n");
		
		if (readInfoTimeOut(buf, (int)sizeof(DISC_RECEIVER)) == -1){
			numberTries=0;
			continue;
		}

		printf("Leu Disc.\n");

		if (equal(buf, DISC_RECEIVER, (int)sizeof(DISC_RECEIVER)) == -1)
			continue;

		printf("Recebeu Disc.\n");

		if (writeInfo(UA_RECEIVER, (int)sizeof(UA_RECEIVER)) == -1){
			continue;
		}

		printf("Escreveu UA.\n");

		return 0;
	}
}



int receiveDISC(int fd){

	char buf[255];
	while(1){

		if (readInfo(buf, (int)sizeof(DISC_TRANSMITTER)) == -1){
			printf("Não leu Disc.\n");
			continue;
		}

		if (equal(buf, DISC_TRANSMITTER, (int)sizeof(DISC_TRANSMITTER)) == -1){
			continue;
		}
		printf("Recebeu Disc.\n");

		if (writeInfo(DISC_RECEIVER, (int)sizeof(DISC_RECEIVER)) == -1){
			printf("Não escreveu Disc.\n");
			continue;
		}
		
		if (readInfo(buf, (int)sizeof(UA_RECEIVER)) == -1){
			continue;
		}

		if (equal(buf, UA_RECEIVER, (int)sizeof(UA_RECEIVER)) == 0){
			printf("Recebeu UA.\n");
			return 0;
		}

	}

}

int ll_close(int fd){
		if(linkL.status==TRANSMITTER){
			sendDISC(fd);
		}
		else if (linkL.status==RECEIVER){
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

void stuffing(char *to_stuff,char *stuffing, int length){

    int i,j=0;
	
   
    for(i=0; i < length; i++){

        if(to_stuff[i]==FLAG){
          stuffing[j]=ESC;
		  j++;
          stuffing[j]=FLAG^STUFF;
        }
        else if(to_stuff[i]==ESC){
          stuffing[j]=ESC;
		  j++;
         stuffing[j]=ESC^STUFF;
        }
        else
          stuffing[j]=to_stuff[i];

        j++;

    }
    stuffing[j]='\0';
}


int buildIFrame(char * stuffed_buffer, int length){

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

	int res, aux=0;
	char * stuffed_buf;
	int header_size = 4; //F A C BCC1 
	int trailer_size = 2; //BCC2 F
	int number_to_stuff = numESC(buffer,length);
	int total_buffer_size = length + number_to_stuff;
	int total_frame_size = length+number_to_stuff + header_size + trailer_size;

	
	stuffed_buf = (char*)malloc(total_buffer_size+1);
	

	stuffing(buffer,stuffed_buf,length+1);

	buildIFrame(stuffed_buf,total_buffer_size);

	while(numberTries < linkL.numTransmissions){

		//verificar
		//manda só o que ainda não enviou
		res = write(fd,linkL.frame + aux ,total_frame_size - aux);
		
		if (res == total_frame_size){
			printf("Enviou frame.\n");
			return 0;
		}

		aux = res;

		if (res == -1)
			aux=0;

	}
	return -1;
}

void destuffing(char *to_destuff,char*destuffing, int length){

	int i;
	int j=0;
	
	for(i=0;i < length;i++){
		if(to_destuff[i]==ESC){
			destuffing[j]= to_destuff[++i]^STUFF;
		}
		else
			destuffing[j]= to_destuff[i];

		j++;
		}
}



int ll_read(int fd, char * buffer, int length){

	char * destuffed_buf;
	destuffed_buf = (char*)malloc(length+1);
	int res;
	unsigned char *byte;

	while(1){

		

	}

}




int main(int argc, char** argv){

	int c, res;
	char buf[]={FLAG,'1','2'};
	int sum = 0, speed = 0;

	//inicializacoes
	linkL.portName = argv[1];
	linkL.timeout = WAIT_TIME;
	linkL.numTransmissions = NR_TRIES;
	linkL.status = TRANSMITTER;

	(void) signal(SIGALRM,alarm_handler);

	if (ll_open(linkL.portName)){
		printf("Falhou ll_open.\n");
		return -1;
	}

	sleep(2);

	/*if(linkL.status==TRANSMITTER)
		ll_write(fd, buf,3);
	else if(linkL.status==RECEIVER)
		ll_read(fd,buf,3);
	else
		{
			printf("Status errado!\n");
			return 1;
		}
*/

	ll_close(fd);

  return 0;
}
