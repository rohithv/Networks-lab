#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdint.h>

int main(){
	int cliSocket;
	char buff[512];
	struct sockaddr_in serverAddr;
	socklen_t addrSize;
	FILE *fc;
	uint32_t file_size, nw_file_size;
	unsigned int n, num_sent = 0;
	
	cliSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(cliSocket < 0){
		printf("Socket creation failed\n");
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
	
	addrSize = sizeof(serverAddr);
	if(connect(cliSocket, (struct sockaddr *)&serverAddr, addrSize) < 0){
		printf("Error connecting\n");
		exit(1);
	}
	
	fc = fopen("RollC.txt", "r");
	
	fseek(fc,0L,SEEK_END);
	file_size= ftell(fc); //Assumes file size is less than 2^32 i.e., 4GB.
	nw_file_size = htonl(file_size);
	fseek(fc,0L,SEEK_SET);
	
	//Sending file size first
	send(cliSocket, &nw_file_size, sizeof(nw_file_size), 0);
	
	//sending data of file in blocks of 512.
	num_sent = 0;
	while(num_sent < file_size){
		n = fread(buff, 1, 512, fc);	// Assuming fread() and send() works as expected
		send(cliSocket, buff, n, 0);	// error checks are not made. Also assuming server never resets the connection
		num_sent+=n;
	}
	fclose(fc);
	
	//receiving data from server
	memset(buff, 0, 512);
	n = recv(cliSocket, buff, 6, 0);
	
	fc = fopen("NumberC.txt", "w+");
	fwrite(buff, 1, 6, fc);
	fclose(fc);
	
	return 0;
}
