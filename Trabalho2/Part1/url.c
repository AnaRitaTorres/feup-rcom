#include "url.h"

const char* reg =	"ftp://([([A-Za-z0-9])*:([A-Za-z0-9])*@])*([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

const char* regAnon = "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

void init_URL(URL_info url){
	memset(url->user, 0, sizeof(url_string));
	memset(url->password, 0, sizeof(url_string));
  url->port = 0;
	memset(url->host, 0, sizeof(url_string));
	memset(url->path, 0, sizeof(url_string));
	memset(url->filename, 0, sizeof(url_string));
}


int parse_URL(URL_info* url, const char* string){

  char* tempURL, *element, *activeExp;//mudar o nome destas variaveis
  int passwordMode;
  regex_t* regExp;
  size_t regExp_size = sizeof(regex_t);
  size_t string_size = strlen(string);
  regmatch_t ptr_match[string_size];


  element = (char*) malloc(string_size);
	tempURL = (char*) malloc(string_size);

	memcpy(tempURL,string,string_size);

	if (tempURL[6] == '[') {
		passwordMode = 1;
		activeExp = (char*) reg;
	}
  else {
		passwordMode = 0;
		activeExp = (char*) regAnon;
	}

  regExp = (regex_t*) malloc(regExp_size);

  int compileExp;

  //Used to compile a regular expression into a form that is suitable for subsequent regexec() searches.
	if ((compileExp = regcomp(regExp, activeExp, REG_EXTENDED)) != 0) {
		printf("URL Did Not Compile!");
		return 1;
	}

  //Compares the null-terminated string specified by string with the compiled regular expression
  //preg initialized by a previous call to regcomp().
	if ((compileExp = regexec(regex, tempURL, string_size, ptr_match, REG_EXTENDED)) != 0) {
		printf("URL Did Not Execute!");
		return 1;
	}

  free(regExp);

  //Removing ftp:// from string
  strcpy(tempURL, tempURL + 6);

  if (passwordMode) {
    //Removing [ from string
    strcpy(tempURL, tempURL + 1);

    // saving username
    strcpy(element, processElementUntilChar(tempURL, ':'));
    memcpy(url->user, element, strlen(element));

    //saving password
    strcpy(element, processElementUntilChar(tempURL, '@'));
    memcpy(url->password, element, strlen(element));
    strcpy(tempURL, tempURL + 1);
  }

//saving host
  strcpy(element, processElementUntilChar(tempURL, '/'));
  memcpy(url->host, element, strlen(element));

  //saving url path
  char* path = (char*) malloc(strlen(tempURL));
  int startPath = 1;
  while (strchr(tempURL, '/')) {
    element = processElementUntilChar(tempURL, '/');

    if (startPath) {
      startPath = 0;
      strcpy(path, element);
    } else {
      strcat(path, element);
    }

    strcat(path, "/");
  }
  strcpy(url->path, path);

  // saving filename
  strcpy(url->filename, tempURL);

  free(tempURL);
  free(element);
}

char* processElementUntilChar(char* str, char chr) {
	// using temporary string to process substrings
	char* tempStr = (char*) malloc(strlen(str));

	// calculating length to copy element
	int index = strlen(str) - strlen(strcpy(tempStr, strchr(str, chr)));

	tempStr[index] = '\0'; // termination char in the end of string
	strncpy(tempStr, str, index);
	strcpy(str, str + strlen(tempStr) + 1);

	return tempStr;
}
