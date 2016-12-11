#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <regex.h>
#include <string.h>
#include <pcre.h>
#include <stdio.h>
#include <string.h>

typedef char url_string[256];

// Structure to store the components of an URL
typedef struct URL_info
{
    url_string user;
    url_string password;
    int port;
    url_string host;
    url_string ip;
    url_string path;
    utl_string filename;
} URL_info;

// Initialization of an empty URL
void init_URL(URL_info url);

//Parse a string with the URL content
int parse_URL(URL_info* url, const char* string);
