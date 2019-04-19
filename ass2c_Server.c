#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>

int main(){
	int welcomeSocket1,welcomeSocket2, cli1, cli2;
	struct sockaddr_in serverAddr1, serverAddr2;
	struct sockaddr_storage cliAddr1, cliAddr2;
	socklen_t addrSize1, addrSize2;
	char buff[15], buff2[15], c;
	//unsigned int num_recv, conn;
	//uint32_t file_size;
	int n, maxsock, r;
	FILE *fs;
	fd_set readfds;
	struct timeval tv;
	
	welcomeSocket1 = socket(PF_INET, SOCK_STREAM, 0);
	if(welcomeSocket1 < 0){
		printf("Socket1 creation failed\n");
		exit(1);
	}

	welcomeSocket2 = socket(PF_INET, SOCK_STREAM, 0);
	if(welcomeSocket2 < 0){
		printf("Socket2 creation failed\n");
		exit(1);
	}
	
	serverAddr1.sin_family = AF_INET;
	serverAddr1.sin_port = htons(5555);
	//serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	serverAddr1.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr1.sin_zero, 0, sizeof(serverAddr1.sin_zero));

	serverAddr2.sin_family = AF_INET;
	serverAddr2.sin_port = htons(5556);
	//serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	serverAddr2.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr2.sin_zero, 0, sizeof(serverAddr2.sin_zero));
	
	if(bind(welcomeSocket1, (struct sockaddr *) &serverAddr1, sizeof(serverAddr1)) < 0){
		printf("Cannot bind1\n");
		exit(1);
	}

	if(bind(welcomeSocket2, (struct sockaddr *) &serverAddr2, sizeof(serverAddr2)) < 0){
		printf("Cannot bind2\n");
		exit(1);
	}
	
	if(listen(welcomeSocket1, 1) != 0){
		printf("Error listening1\n");
		exit(1);
	}

	if(listen(welcomeSocket2, 1) != 0){
		printf("Error listening2\n");
		exit(1);
	}
	printf("Listening on 1&2\n");
	
	addrSize1 = sizeof(cliAddr1);
	addrSize2 = sizeof(cliAddr2);
	cli1 = accept(welcomeSocket1, (struct sockaddr *)&cliAddr1, &addrSize1);
	cli2 = accept(welcomeSocket2, (struct sockaddr *)&cliAddr2, &addrSize2);
	printf("Accepted two connections\n");
	
	memset(buff, 0, 15);
	
	/* Using select() for checking whether data is available in any of the sockets.
	 * Alternative1: use poll() : bad idea
	 * Alternative2: make the two sockets non blocking and set a timeout value and keep on recv()ing from both the sockets alternatively
	 * NOTE: FCFS is not guaranteed.
	 */
	
	FD_ZERO(&readfds);
	FD_SET(cli1, &readfds);
	FD_SET(cli2, &readfds);
	maxsock = cli1>cli2 ? cli1 : cli2;
	tv.tv_sec=2;
	tv.tv_usec=500000; //timeout = 2.5sec
	 
	while(1){
		FD_ZERO(&readfds);
		FD_SET(cli1, &readfds);
		FD_SET(cli2, &readfds);
		maxsock = cli1>cli2 ? cli1 : cli2;
		maxsock++;
		tv.tv_sec=2;
		tv.tv_usec=500000; //timeout = 2.5sec
		r = select(maxsock, &readfds, NULL, NULL, &tv);
		if(r<0){
			printf("select failed\n");
			exit(2);
		}
		if(r ==0) //timeout
			continue;
		//else
		if(cli1!=-1 && FD_ISSET(cli1, &readfds)){
			n = recv(cli1, &c, 1, 0); //receive the character
			fs = fopen("RollS.txt", "r+");
			memset(buff, 0, 15);
			fseek(fs, 1L, SEEK_SET);
			n = fread(buff, 1, 6, fs);
			if(n<6){
				if(feof(fs)){
					close(cli1);
					FD_CLR(cli1, &readfds);
					if(cli2!=-1)
						maxsock = cli2;
					else
						break;
					cli1 = -1;
				}
			}
			send(cli1, buff, 6, 0);
			//delete_line(fs); //function to delete first line from file.
			//delete code
			char tmp[] = "temp.txt";
			//char buff[15];
			FILE *temp = fopen(tmp, "w+");
			fseek(fs,0L, SEEK_SET);
			fscanf(fs,"%s",buff);
	
			while(fscanf(fs,"%s",buff) > 0){
				fprintf(temp, "%s\n",buff);
			}
			fclose(fs);
			fclose(temp);
			remove("RollS.txt");
			rename("temp.txt", "RollS.txt");
			//fclose(fs);
		}
		if(cli2!=-1 && FD_ISSET(cli2, &readfds)){
			n = recv(cli2, &c, 1, 0); //receive the character
			fs = fopen("RollS.txt", "r+");
			memset(buff, 0, 15);
			fseek(fs, 1L, SEEK_SET);
			n = fread(buff, 1, 6, fs);
			if(n<6){
				if(feof(fs)){
					close(cli2);
					FD_CLR(cli2, &readfds);
					if(cli1!=-1)
						maxsock = cli1;
					else
						break;
					cli2 = -1;
				}
			}
			send(cli2, buff, 6, 0);
			//delete_line(fs);
			//delete code
			char tmp[] = "temp.txt";
			//char buff[15];
			FILE *temp = fopen(tmp, "w+");
			fseek(fs, 0L, SEEK_SET);
			fscanf(fs,"%s",buff);
	
			while(fscanf(fs,"%s",buff) > 0){
				fprintf(temp, "%s\n",buff);
			}
			fclose(fs);
			fclose(temp);
			remove("RollS.txt");
			rename("temp.txt", "RollS.txt");
			//fclose(fs);
			fclose(fs);
		}
	}
	
	return 0;
}
/*
void delete_line(FILE *fs){
	char tmp[] = "temp.txt";
	char buff[15];
	FILE *temp = fopen(tmp, "w+");
	fseek(fs,0L, SEEK_SET);
	fscanf(fs,"%s",buff);
	
	while(fscanf(fs,"%s",buff) > 0){
		fprintf(temp, "%s\n",buff);
	}
	fclose(fs);
	fclose(temp);
	remove("RollS.txt");
	rename("temp.txt", "RollS.txt");
}*/
