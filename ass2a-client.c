#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(){
	int cliSocket;
	char buff1[1024], buff2[1024];
	struct sockaddr_in serverAddr;
	socklen_t addrSize;
	
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
	
	while(1){
		printf(">");
		memset(buff1, 0, 1024);
		scanf("%[^\n]s",buff1);
		getchar();
		send(cliSocket, buff1, 1024, 0);
		printf("sent: %s\n",buff1);
		memset(buff2, 0, 1024);
		recv(cliSocket, buff2, 1024, 0);
		printf("received: %s\n",buff2);
		if(!strcmp(buff2, "This number belongs to NITC S6/S8 batch"))
			break;
	}
	return 0;
}
