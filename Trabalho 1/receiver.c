/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

/* Definitions */
#define FLAG   0x7E
#define A      0x03
#define C_SET  0x03
#define C_UA   0x07
#define NR_TRIES  3
#define WAIT_TIME 3

/* Global/Const variables */
const unsigned char SET[] = {FLAG, A, C_SET, A^C_SET, FLAG};
const unsigned char UA[] = {FLAG, A, C_UA, A^C_UA, FLAG};
int filedes, numberTries = 0;

volatile int STOP=FALSE;
int flag=1;
void atende()                   // atende alarme
{
	printf("alarme # %d\n", numberTries);
	flag=1;
	numberTries++;
}


void receiveSET(int fd){

	int res;
	char buf[255];

	while(numberTries < NR_TRIES){

		res =read(fd, buf,sizeof(SET));

		if(flag){
		  alarm(WAIT_TIME);                 // activa alarme de 3s
		  flag=0;
	   	}
   	 
    
	    	if(res < sizeof(SET))
		   continue;

		int success=1;
		if(res == sizeof(SET))
		{ 

			printf("buf: \n"); 
			int i;
			for(i=0; i < sizeof(SET);i++)
			{

		  		if(buf[i]!= SET[i])
				{
				   success=0;
				   break;
				}
				printf("%02x\n",buf[i]);
			}
			if (success == 1)
			{	
				write(fd,UA,sizeof(UA));
			}
		
		}
	
	
	}

}


int main(int argc, char** argv)
{
    int fd,c, res,success;
    struct termios oldtio,newtio;
    char buf[255];

    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    	printf("New termios structure set\n");

	receiveSET(fd);
	
	/*int aux = 0;


	//Receção de informação
	while (1){
		res = read(fd, buf+aux, 1);
		
		if (buf[aux] == '\0')
			break;
		
		aux++;
	}

	printf("%s\n", buf);

	//reenvio de informação
	aux=0;

	while (1){

		res = write(fd, buf + aux, 1);

		if (buf[aux] == '\0')
			break;

		aux++;

	}*/


  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */

	sleep(2);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
