#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "url.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Number of Arguments is Invalid!\n");
		return 1;
	}

	//URL

	URL_info url;
	initURL(&url);

  return 0;

}
