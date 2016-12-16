#include "FTP.h"

static int connectSocket(const char* ip, int port) {
	int sockfd;
	struct sockaddr_in server_addr;

	// server address handling
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port); /*server TCP port must be network byte ordered */

	// open an TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return -1;
	}

	// connect to the server
	if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr))
			< 0) {
		perror("connect()");
		return -1;
	}

	return sockfd;
}

int ftpConnect(ftp* ftp, const char* ip, int port) {

	int socket_fd;
	char buffer[SIZE];

	socket_fd = connectSocket(ip, port);

	if (socket_fd == -1){
		printf("Can't Connect Socket!\n");
		return 1;
	}

	ftp->control_fd = socket_fd;
	ftp->data_fd = 0;

	size_t buffer_size = sizeof(buffer);

	if (ftpRead(ftp, buffer, buffer_size) !=0) {
		printf("ftpRead Failed!(ftpConnect)\n");
		return 1;
	}

	return 0;
}

int ftpLogin(ftp* ftp, const char* user, const char* password) {

	char buffer[SIZE];

	//Username
	sprintf(buffer, "USER %s\r\n", user);

	size_t buffer_size1 = strlen(buffer);

	if (ftpWrite(ftp, buffer, buffer_size1)!=0) {
		printf("ftpWrite Failed!(ftpLogin1)\n");
		return 1;
	}

	size_t buffer_size2 = sizeof(buffer);

	if (ftpRead(ftp, buffer, buffer_size2)!=0) {
		printf("ftpRead Failed!(ftpLogin1)\n");
		return 1;
	}

	size_t buffer_size3 = sizeof(buffer);

	// Cleaning Buffer
	memset(buffer, 0, buffer_size3);

	// Password
	sprintf(buffer, "PASS %s\r\n", password);

	size_t buffer_size4 = sizeof(buffer);

	if (ftpWrite(ftp, buffer, buffer_size4)!=0) {
		printf("ftpWrite Failed!(ftpLogin2)\n");
		return 1;
	}

	size_t buffer_size5 = sizeof(buffer);

	if (ftpRead(ftp, buffer, buffer_size5)!=0) {
		printf("ftpRead Failed!(ftpLogin2)\n");
		return 1;
	}

	return 0;
}

int ftpChangeDir(ftp* ftp, const char* path) {

	char buffer[SIZE];

	sprintf(buffer, "CWD %s\r\n", path);

	size_t buffer_size1=strlen(buffer);

	if (ftpWrite(ftp, buffer, buffer_size1) !=0) {
		printf("ftpWrite Failed!(ftpChangeDir)\n");
		return 1;
	}

	size_t buffer_size2=sizeof(buffer);

	if (ftpRead(ftp, buffer, buffer_size2)!=0) {
		printf("ftpRead Failed!(ftpChangeDir)\n");
		return 1;
	}

	return 0;
}

int ftpPassive(ftp* ftp) {

	char buffer[SIZE] = "PASV\r\n";
	size_t buffer_size1 = strlen(buffer);

	if (ftpWrite(ftp, buffer, buffer_size1)!=0){
		printf("ftpWrite Failed!(ftpPassive)\n");
		return 1;
	}

	size_t buffer_size2 = sizeof(buffer);

	if (ftpRead(ftp, buffer, buffer_size2)!=0){
		printf("ftpRead Failed!(ftpPassive)\n");
		return 1;
	}

	int ip[4];
	int port[2];

	if ((sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip[0],
			&ip[1], &ip[2], &ip[3], &port[0], &port[1])) < 0) {
		printf("Error Calculating Port!\n");
		return 1;
	}

	size_t buffer_size3 = sizeof(buffer);

	memset(buffer, 0, buffer_size3);

	if ((sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]))< 0) {
		printf("Ip Address Incorrectly Formed!\n");
		return 1;
	}

	int portResult = port[0] * 256 + port[1];

	printf("IP: %s\n",buffer);
	printf("PORT: %d\n", portResult);

	ftp->data_fd = connectSocket(buffer, portResult);

	if (ftp->data_fd == -1) {
		printf("Socket Descriptor is Wrong!\n");
		return 1;
	}

	return 0;
}

int ftpRetrieve(ftp* ftp, const char* filename) {
	char buffer[SIZE];

	sprintf(buffer, "RETR %s\r\n", filename);

	size_t buffer_size1= strlen(buffer);

	if (ftpWrite(ftp, buffer, buffer_size1)!=0) {
		printf("ftpWrite Failed!(ftpRetrieve)\n");
		return 1;
	}

	size_t buffer_size2= strlen(buffer);

	if (ftpRead(ftp, buffer, buffer_size2)) {
		printf("ftpRead Failed!(ftpRetrieve)\n");
		return 1;
	}

	return 0;
}

int ftpDownload(ftp* ftp, const char* filename) {

	
	int bytes;
	char buffer[SIZE];

	FILE* file = fopen(filename, "w");
	if (!file){
		printf("Can't Open File!\n");
		return 1;
	}


	while ((bytes = read(ftp->data_fd, buffer, sizeof(buffer)))) {
		if (bytes < 0) {
			return 1;
		}

		if ((bytes = fwrite(buffer, bytes, 1, file)) < 0) {
				return 1;
		}
	}

	fclose(file);
	close(ftp->data_fd);

	return 0;
}

int ftpDisconnect(ftp* ftp) {
	char disc[SIZE];

	if (ftpRead(ftp, disc, sizeof(disc))) {
		printf("ERROR: Cannot disconnect account.\n");
		return 1;
	}

	sprintf(disc, "QUIT\r\n");
	if (ftpWrite(ftp, disc, strlen(disc))) {
		printf("ERROR: Cannot send QUIT command.\n");
		return 1;
	}

	if (ftp->control_fd)
		close(ftp->control_fd);

	return 0;
}

int ftpWrite(ftp* ftp, const char* str, size_t size) {
	int bytesWritten;

	bytesWritten = write(ftp->control_fd, str, size);

	if(bytesWritten <= 0){
		printf("ftpWrite Failed!\n");
		return 1;
	}
	return 0;
}

int ftpRead(ftp* ftp, char* str, size_t size) {
	FILE* fp = fdopen(ftp->control_fd, "r");

	do {
		memset(str, 0, size);
		str = fgets(str, size, fp);
		printf("%s", str);
	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}
