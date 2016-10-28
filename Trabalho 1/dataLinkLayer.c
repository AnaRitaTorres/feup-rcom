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
	int aux;
	unsigned char buf[255];
	
	while(numberTries < linkL.numTransmissions){

		if (writeInfo(SET, (int)sizeof(SET)) == -1){
			continue;
		}
		printf("Escreveu SET.\n");

		aux=numberTries+1;
		if (readInfoTimeOut(buf, (int)sizeof(UA_TRANSMITTER)) == -1){
			numberTries=aux; //acabar se não der ou reenviar??
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
	
	int aux;
	char buf[255];
	numberTries=0;

	while(numberTries < linkL.numTransmissions){

		if (writeInfo(DISC_TRANSMITTER, (int)sizeof(DISC_TRANSMITTER)) == -1){
			continue;
		}

		printf("Escreveu Disc.\n");
		
		aux=numberTries+1;
		if (readInfoTimeOut(buf, (int)sizeof(UA_TRANSMITTER)) == -1){
			numberTries=aux; //acabar se não der ou reenviar??
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
			continue;
		}

		if (equal(buf, DISC_TRANSMITTER, (int)sizeof(DISC_TRANSMITTER)) == -1){
			continue;
		}
		printf("Recebeu Disc.\n");

		if (writeInfo(DISC_RECEIVER, (int)sizeof(DISC_RECEIVER)) == -1){
			continue;
		}
		printf("Escreveu Disc.\n");
		
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


int buildIFrame(char * stuffed_packet,unsigned char a, unsigned char c, int length){

	//without stuffing
	int i, bcc2;
	
	linkL.frame[0] = FLAG;
	linkL.frame[1] = a;
	linkL.frame[2] = c;	
	linkL.frame[3] = a^c;  //ver se é necessário cópia para guardar dados
	
	for (i=0; i < length; i++){
		linkL.frame[i+4] = stuffed_packet[i];

		if(i==0)
			bcc2 = stuffed_packet[i];
		else
			bcc2 ^= stuffed_packet[i];
	}

	linkL.frame[i+4] = bcc2;
	linkL.frame[i+5] = FLAG;

	return 0;

}

int buildFrame(unsigned char a, unsigned char c){

	
	linkL.frame[0] = FLAG;
	linkL.frame[1] = a;
	linkL.frame[2] = c;	
	linkL.frame[3] = a^c;  
	linkL.frame[4] = FLAG;

	return 0;
}

int ll_write(int fd, char * packet, int length){

	int res;
	char buffer[MAX_SIZE];
	char * stuffed_packet;
	int header_size = 4; //F A C BCC1 
	int trailer_size = 2; //BCC2 F
	int number_to_stuff = numESC(packet,length);
	int total_buffer_size = length + number_to_stuff;
	int total_frame_size = length+number_to_stuff + header_size + trailer_size;
	unsigned char a=A_TRANSMITTER, c=(linkL.sequenceNumber << 6);

	printf("entrou na llwrite\n");
	stuffed_packet = (char*)malloc(total_buffer_size+1);
	
	stuffing(packet,stuffed_packet,length+1);

	buildIFrame(stuffed_packet,a,c,total_buffer_size);

	while(numberTries < linkL.numTransmissions){

		res = write(fd,linkL.frame,total_frame_size);
		
		if (res == total_frame_size){
			printf("Enviou frame.\n");
			break;
		}
	
	}

	int t=receiveFrame(fd,linkL.frame,A_RECEIVER,C_RR|| C_REJ);
	int x;
	
	for(x=0;x < t;x++){
		printf("%02x\n",linkL.frame[x]);
	}

	return -1;
}

int destuffing(char *to_destuff,char*destuffing, int length){

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

		return j;
}

int receiveFrame(int fd, char * frame, unsigned char a, unsigned char c){
	
	unsigned char byte;
	int res,i=0;
	State s = START_RCV;
	

	while(s!=STOP_RCV){
		res=read(fd,&byte,1);

		if(res!=1){
			continue;
		}

		switch(s){

			case START_RCV:
				if (byte == FLAG){
					frame[i++] = byte;
					s = FLAG_RCV;
				}
				break;

             case FLAG_RCV:
                if (byte == a){
                    frame[i++] = byte;
                    s = A_RCV;
                }
                else if (byte == FLAG);
                else{
                        s = START_RCV;
						i=0;
                }
                break;

				case A_RCV:
                    if (byte == FLAG){
						s = FLAG_RCV;
						i=1;
                    }
                    else if (byte==c){
							frame[i++]=byte;
							s=C_RCV;
					}
					else{
						s=START_RCV;
						i=0;
					}
                    break;

                case C_RCV:
                    if (byte == FLAG){
                            s=FLAG_RCV;
							i=1;
                    }
                    else if(byte==(frame[1]^frame[2])){
                            frame[i++]=(frame[1]^frame[2]);
							s = BCC_OK;
                    }
					else{
						s=START_RCV;
						i=0;
					}
                    break;

                case BCC_OK:
                    if (byte == FLAG){
							frame[i++] = byte;
							s = STOP_RCV;
                    }
                    else{
                        s=START_RCV;
						i=0;

                    }
                    break;

                default:
                	break;
                }
		}
		return i;
}

int receiveIFrame(int fd, char * frame){
	
	unsigned char byte, bcc2=0;
	int res,i=0;
	State s = START_RCV;
	

	while(s!=STOP_RCV){

		res=read(fd,&byte,1);
		
		if(res!=1){
			printf("Não leu o byte.\n");
			break;
		}

		switch(s){

			case START_RCV:
				if (byte == FLAG){
					frame[i++] = byte;
					s = FLAG_RCV;
				}
				break;

             case FLAG_RCV:
                if (byte == A_TRANSMITTER){
                    frame[i++] = byte;
                    s = A_RCV;
                }
                else if (byte == FLAG);
                else{
                        s = START_RCV;
						i=0;
                }
                break;

				case A_RCV:
                    if (byte == FLAG){
						s = FLAG_RCV;
						i=1;
                    }
                    else if (byte==C_S0){
							frame[i++]=byte;
							s=C_RCV;
					}
					else if(byte==C_S4){
							frame[i++]=byte;
							s=C_RCV;
                    }
					else{
						s=START_RCV;
						i=0;
					}
                    break;

                case C_RCV:
                    if (byte == FLAG){
                            s=FLAG_RCV;
							i=1;
                    }
                    else if(byte==(frame[1]^frame[2])){
                            frame[i++]=(frame[1]^frame[2]);
							s = BCC_OK;
                    }
					else{
						s=START_RCV;
						i=0;
					}
                    break;

                case BCC_OK:
                    if (byte == FLAG){
							frame[++i] = byte;
							s = STOP_RCV;
                    }
                    else{
                        frame[i++] = byte;
						bcc2^=byte;

                    }
                        break;

                default:
                        break;
                }
		}
		frame[--i]=bcc2;
		return i+1;
}

int getPacket(char * buffer,char * packet, int buffer_size){

	int i,j=0;
	for(i=4; i < buffer_size-2;i++,j++){
		packet[j]=buffer[i];
	}

	return j;
}

int ll_read(int fd, char * buffer, int buffer_size){

	char * destuffed_packet, * packet;
	unsigned char c;
	int to_destuff_size, destuffed_packet_size, sequence, packet_size, bcc2_aux = buffer[buffer_size-2], bcc1_aux = buffer[3];
			
	to_destuff_size= receiveIFrame(fd,buffer);
	
	destuffed_packet = (char*)malloc(to_destuff_size);
	packet = (char*)malloc(to_destuff_size-6);

	packet_size=getPacket(buffer,packet,to_destuff_size);

	destuffed_packet_size=destuffing(packet,destuffed_packet,packet_size);
	
	sequence = (destuffed_packet[2] >> 6);

	if(linkL.frame[3]!=bcc1_aux || linkL.frame[packet_size-2]!=bcc2_aux){

		if(sequence==0){
			c = (0 << 7) | C_REJ;
			buildFrame(A_RECEIVER,c);
			write(fd,linkL.frame,5);
		}
		else if(sequence==1){
			c = (1 << 7) | C_REJ;
			buildFrame(A_RECEIVER,c);
			write(fd,linkL.frame,5);
		}
	}
	else{

		if(sequence == 0){
			c = (1 << 7) | C_RR;
			buildFrame(A_RECEIVER,c);
			write(fd,linkL.frame,5);
		}
		else if(sequence == 1){
			c = (0 << 7) | C_RR;
			buildFrame(A_RECEIVER,c);
			write(fd,linkL.frame,5);
		}

	}
	int x;
	for(x=0; x < 5;x++){
		printf("%02x\n",linkL.frame[x]);
	}

	if(linkL.sequenceNumber==1){
		linkL.sequenceNumber=0;
	}
	else if(linkL.sequenceNumber==0){
		linkL.sequenceNumber=1;
	}

	return 0;

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

	if(linkL.status==TRANSMITTER)
		ll_write(fd, buf,3);
	else if(linkL.status==RECEIVER)
		 ll_read(fd,linkL.frame,3);
	else{
			printf("Status errado!\n");
			return 1;
		}


	ll_close(fd);

  return 0;
}
