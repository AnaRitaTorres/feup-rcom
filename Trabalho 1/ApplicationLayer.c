#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <ApplicationLayer.h>
#include <dataLinkLayer.h>
#include <unistd.h>
#include <stdio.h>

#define SIZE_LIMIT (INPUT_MAX_SIZE - OSIZE)

unsigned int fileSize;
unsigned int counter = 0;
const char *fileName
struct Packet dataPacket;
struct Packet startPacket;

int fileRead(const char *fileName, char *content)
{

FILE *file;

file = fopen(fileName, "rb");

if (file == NULL)
	{
		printf("Error opening file!\n");
		exit(-1);
	}	

fseek(file,0,SEEK_END);

fileSize = ftell(file);

fseek(file,0,SEEK_SET);

content = (char *) malloc(fileSize);

if (content == NULL)
{
exit(-1);
}


fread(content, fileSize, sizeof(char), file);

fclose(file);

return 0;

}


int fileWrite(FILE *file, char *buffer, int length)
{
	int i;
	
	for (i = 0; i < length; i++)
		{
			fprintf(file, "%c", buffer[i]);
		}
	
}

int getControlPacket(struct Packet *pckt, char C_FLAG)

{
	packet->size = 8;
	packet->frame = (char *) malloc(packet->size);
	if(packet->frame == NULL)
		{
			return -1
		}
	packet->frame[0] = C_FLAG;
	packet->frame[1] = fileSize;
	packet->frame[3] = sizeof(fileSize);
	memcpy(&packet->frame[3], fileSize, sizeof(fileSize));
	
	return 0;
}

//str -> chamada da funcao fileRead(fileName, str);

int getDataPacket(char *content, char *str, int strLength)
{
	unsigned int nrPackets;
	unsigned int sent = 0;
	unsigned int bytesLeft = fileSize;
	
	nrPackets = ceil(float) fileSize / SIZE_LIMIT
	
	content[0] = 1;
	content[1] = counter++;
	content[2] = (strLength >> 8) & 0xFF;
	counter[3] = strLength & 0xFF;
	memcpy(&content[OSIZE], str, strLength);
	
	return 0;
}


