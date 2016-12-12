#pragma once

#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <ctype.h>

typedef char url_size[128];

typedef struct URL {
	const char * urlToParse;	// string to url
	url_size user; 			// string to user
	url_size password; 		// string to password
	url_size host; 			// string to host
	url_size ip; 			// string to IP
	url_size path; 			// string to path
	url_size filename; 		// string to filename
	int hasUser;
	int port; 			// integer to port
} url;

void initURL(url* url, const char* urlToParse);
int parseURL(url* url); // Parse a string with the url to create the URL structure
int getIpByHost(url* url); // gets an IP by host name

char* processElementUntilChar(char* str, char chr);
