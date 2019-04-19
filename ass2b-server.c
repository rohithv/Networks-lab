#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define ESTABLISHED 1
#define CLOSED 0

int main(){
	int welcomeSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage cliAddr;
	socklen_t addrSize;
	char buff[512];
	unsigned int num_recv, conn;
	uint32_t file_size;
	int n;
	FILE *fs;
	
	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(welcomeSocket < 0){
		printf("Socket creation failed\n");
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);
	//serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
	
	if(bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
		printf("Cannot bind\n");
		exit(1);
	}
	
	if(listen(welcomeSocket, 5) != 0){
		printf("Error listening\n");
		exit(1);
	}
	printf("Listening\n");
	
	while(1){ //infinite loop for server
		printf("Waiting for new connection\n");
		addrSize = sizeof(cliAddr);
		newSocket = accept(welcomeSocket, (struct sockaddr *)&cliAddr, &addrSize);
		conn= ESTABLISHED;
		
		memset(buff, 0, 512);
		//read filesize
		recv(newSocket, &file_size, sizeof(file_size), 0);
		file_size = ntohl(file_size);
		
		//receive the data and write to a file
		fs = fopen("RollS.txt", "w+");
		num_recv = 0;
		while(num_recv < file_size){
			n = recv(newSocket, buff, 512, 0);
			if(n<=0){
				printf("client closed the connection\n");
				conn=CLOSED;
				break;
			}
			fwrite(buff, 1, n, fs); //assuming fwrite writes all n bytes to the file (as of now)
			num_recv += n;
		}
		
		if(conn==CLOSED){ //if client closes connection in between file transfer
			close(newSocket);
			fclose(fs);
			continue;
		}
		
		fseek(fs, 1L, SEEK_SET);
		memset(buff, 0, 512);
		fread(buff, 1, 6, fs);// Again trusting read to read 6 bytes (no error checks are made)
		send(newSocket, buff, 6, 0); //similar trust
		
		close(newSocket);
		conn=CLOSED;
		fclose(fs);
	}
	
	return 0;
}
	
