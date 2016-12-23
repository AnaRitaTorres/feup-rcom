#include "URL.h"

void initURL(url* url, const char* urlToParse) {

	//Copies URL To urlToParse
	url->urlToParse = (const char*)malloc(strlen(urlToParse));
	url->urlToParse = urlToParse;

	//Sets variables to 0, deletes any possible trash
	memset(url->user, 0, sizeof(url_size));
	memset(url->password, 0, sizeof(url_size));
	memset(url->host, 0, sizeof(url_size));
	memset(url->path, 0, sizeof(url_size));
	memset(url->filename, 0, sizeof(url_size));
	
	//Connect socket (control socket) to ftp server on the port 21.
	url->port = 21;
}

char * getStringBeforeChar(char * string, char symbol){

	char *result = (char *) malloc (strlen(string));

	strcpy(result, string);

	char * auxiliar = strchr(result, symbol);


	if(auxiliar == NULL){
		return NULL;
	}

	int index = (int)(auxiliar - result);

	result[index] = '\0';
	
	return result;
}

int checkIfValid(char * string){
	int valid = 0;
	int i;
	for (i=0; i< strlen(string); i++){
		if(!(isdigit(string[i]) || isalpha(string[i]) || string[i] == '+' || string[i] == '.' || string[i] == '-')){
			valid = -1;
			printf("\n%c\n",string[i]);
			break;
		}
	}

	return valid;
}

int parseURL(url * url){

	int okay = 0;

	//Check if url has user or if it is anonymous
	if(url->urlToParse[6] == '['){
		url->hasUser = 1;
	}
	else{
		url->hasUser = 0;
	}

	//Remove protocol 'ftp' from string and make a copy of it
	char * tmpURL = (char *) malloc(strlen(url->urlToParse)-5);
	strcpy(tmpURL, url->urlToParse+6);
	tmpURL[strlen(tmpURL)]='\0';

	if (url->hasUser){
		
		//get url without '['
		char * auxiliar = (char *) malloc(strlen(tmpURL)-1);
		strcpy(auxiliar, tmpURL+1);
		
		//find '@'
		auxiliar = getStringBeforeChar(auxiliar, '@');

		//get password
		char * password = strchr(auxiliar, ':');
		
		if (password == NULL){
			printf("Error: URL does not contain ':'\n");
			okay = -1;
		}

		if (checkIfValid(password) == -1){
			printf("Error: Invalid password\n");
			okay = -1;
		}
		else
			strcpy(url->password, password+1);

		//get username
		auxiliar = getStringBeforeChar(auxiliar, ':');

		if (checkIfValid(auxiliar) == -1){
			printf("Error: Invalid user\n");
			okay = -1;
		}
		else 
			strcpy(url->user, auxiliar);

		char * hostAndName = strchr(tmpURL, ']');
		strcpy(tmpURL, hostAndName+2);
		
		free(auxiliar);
	}

			
	//Get host
	char *host = getStringBeforeChar(tmpURL, '/');

	if (checkIfValid(host) == -1){
		printf("Error: Invalid host\n");
		okay = -1;
	}
	else 
		strcpy(url->host, host);



	//Get Path
	char *totalPath = strchr(tmpURL+1 , '/');
	char *path = getStringBeforeChar(totalPath+1, '/');

	while(path != NULL){
		strcat(url->path, "/");

		if (checkIfValid(path) == -1){
			printf("Error: Invalid path\n");
			okay = -1;
		}
		else 
			strcat(url->path, path);
		
		totalPath = strchr(totalPath + 1, '/');
		path = getStringBeforeChar(totalPath+1, '/');
	}	
	strcat(url->path, "/");
	

	//Get name of file
	strcpy(url->filename, totalPath+1);
	url->filename[strlen(totalPath)] = '\0';

	if (checkIfValid(url->filename) == -1){
		printf("Error: Invalid filename\n");
		okay = -1;
	}
	
	
	free(tmpURL);

	if (okay == -1)
		return -1;

	else 
		return 0;

}


int getIpByHost(url* url) {
	struct hostent* h;

	if ((h = gethostbyname(url->host)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	// Takes a network address (in a in_addr struct) and converts it into a Dots-And-Numbrs format (NTOA == Network To ASCII)
	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(url->ip, ip);

	return 0;
}

