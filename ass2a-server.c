#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
	int welcomeSocket, newSocket;
	struct sockaddr_in serverAddr, cliAddr;
	socklen_t addrSize;
	char buff1[1024], buff2[1024];
	
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
	
	while(1){
		addrSize = sizeof(cliAddr);
		newSocket = accept(welcomeSocket, (struct sockaddr *)&cliAddr, &addrSize);
		
		memset(buff1, 0, 1024);
		while(recv(newSocket, buff1, 1024, 0) > 0){
			printf("received: %s\n", buff1);
			if(!strcmp(buff1, "Hello:B150541CS")){
				memset(buff2, 0, 1024);
				strcpy(buff2, "This number belongs to NITC S6/S8 batch");
				send(newSocket, buff2, 1024, 0);
			} else{
				memset(buff2, 0, 1024);
				strcpy(buff2, "Ok");
				send(newSocket, buff2, 1024, 0);
			}
			printf("sent: %s\n",buff2);
			memset(buff1, 0, 1024);
		}
		close(newSocket);
	}
	
	return 0;

}
