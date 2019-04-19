#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdint.h>
#include<unistd.h>

int main(){
	int cliSocket;
	char buff[15];
	struct sockaddr_in serverAddr;
	socklen_t addrSize;
	FILE *fc;
	//uint32_t file_size, nw_file_size;
	unsigned int n, num_sent = 0;
	char c;
	
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
		c = getchar();
		if(c!='\n')
			continue;
		
		send(cliSocket, &c, sizeof(c), 0);
		memset(buff, 0, 15);
		n = recv(cliSocket, buff, 6, 0);
		
		if(n<=0)
			break; //connection closed by server.
		
		fc=fopen("RollNumberC1.txt","a+");
		fprintf(fc,"%s\n",buff);
		fclose(fc);
	}
	
	close(cliSocket);
	return 0;
}
