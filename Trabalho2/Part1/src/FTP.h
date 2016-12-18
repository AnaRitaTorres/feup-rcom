#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SIZE 1024

typedef struct FTP
{
    int control_fd; // file descriptor to control socket
    int data_fd; // file descriptor to data socket
} ftp;



int ftpLogin(ftp* ftp, const char* user, const char* password);
int ftpChangeDir(ftp* ftp, const char* path);
int ftpRetrieve(ftp* ftp, const char* filename);
int ftpPassive(ftp* ftp);
int ftpDownload(ftp* ftp, const char* filename);
int ftpConnect(ftp* ftp, const char* ip, int port);
int ftpDisconnect(ftp* ftp);
int ftpWrite(ftp* ftp, const char* str, size_t size);
int ftpRead(ftp* ftp, char* str, size_t size);
