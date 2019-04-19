#include<iostream>
#include<cstdlib>
#include<cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVERPORT 20021
#define FILEPORT 20020
#define MSGPORT 20023

using namespace std;

int main(){
	time_t current_time;
	struct tm * time_info;
	int welcomesocket, cli, climsg, clifile;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage cliAddr;
	socklen_t addrsize;
	int n;
	char buff4[512];
	
	welcomesocket = socket(PF_INET, SOCK_STREAM, 0);
	if(welcomesocket < 0){
		cout<<"Socket1 creation failed"<<endl;
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
	
	if(bind(welcomesocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
		cout<<"Cannot bind1"<<endl;
		exit(1);
	}
	
	if(listen(welcomesocket, 10) != 0){
		cout<<"Error listening1"<<endl;
		exit(1);
	}
	cout<<"Listening for connections"<<endl;
	
	while(1){
		addrsize=sizeof(cliAddr);
		cli= accept(welcomesocket, (struct sockaddr *)&cliAddr, &addrsize);
		cout<<"Received a connection"<<endl;
		
		int pid = fork();
		if(pid==0){ //Child process
			cout<<"Child created"<<endl;
			close(welcomesocket);
			//Control line
			char buff[50];
			while((n=recv(cli, buff, 50, 0)) > 0){ //while we receive requests
				cout<<"Received: "<<buff;
				char buff1[50];
				char *token;
				strncpy(buff1, buff, 50);
				token = strtok(buff1, " ");
				
				if(strcmp(buff, "GOODBYE SERVER\n")==0){//Goodbye
					char buff3[100];
					strcpy(buff3, "THANK YOU, ALL CONNECTIONS SUCCESSFULLY TERMINATED");
					send(cli, buff3, 50, 0);
					//Do Something here
					close(cli);
					cout<<"exiting after goodbye"<<endl;
					exit(0);
				}
				
				if(strcmp(token, "RETR")==0){ //RETR FILE
					FILE *fs;
					token = strtok(NULL, " \n");
					cout<<"File: "<<token<<endl;
					fs = fopen(token,"r");
					if(fs==NULL){ //error opening file
						int port=MSGPORT;
						
						int msglisten;
						msglisten = socket(PF_INET, SOCK_STREAM, 0);
						if(msglisten<0){
							cout<<"msg socket creation failed"<<endl;
						}
						serverAddr.sin_family = AF_INET;
						serverAddr.sin_port = htons(port);
						serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
						memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
						
						if(bind(msglisten, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
							cout<<"Cannot bind to msg socket"<<endl;
							exit(1);
						}
						if(listen(msglisten, 1) != 0){
							cout<<"Error listening in msg socket"<<endl;
							exit(1);
						}
						
						char buff2[20];
						sprintf(buff2, "CONN %d",port);
						send(cli, buff2, 20, 0);
						
						int pid1 = fork();
						if(pid1==0){ //child = message sending process.
							close(cli);
							cout<<"message sending process created"<<endl;
							addrsize=sizeof(cliAddr);
							climsg= accept(msglisten, (struct sockaddr *)&cliAddr, &addrsize);
							close(msglisten);
							char buff3[100];
							time(&current_time);
							time_info = localtime(&current_time);
							strftime(buff3, sizeof(buff3), "%H:%M:%S", time_info);
							strcat(buff3,": FILE NOT FOUND AT CURRENT WORKING DIRECTORY");
							//sprintf(buff3,"FILE NOT FOUND AT CURRENT WORKING DIRECTORY");
							send(climsg, buff3, 100, 0);
							//sleep(2);
							while(recv(climsg, buff3, 100, 0) > 0); //wait till client closes the connection first
							close(climsg);
							cout<<"msg sending process exiting"<<endl;
							exit(0);
							cout<<"error in msg sending process exiting"<<endl;
						}
						else{ //parent
							close(msglisten);
							cout<<"parent waiting for new commands"<<endl;
							continue; //listens for more commands
						}
					
					}
					else{ //file is present
						int port=FILEPORT;
						int filelisten;
						filelisten = socket(PF_INET, SOCK_STREAM, 0);
						if(filelisten<0){
							cout<<"msg socket creation failed"<<endl;
						}
						serverAddr.sin_family = AF_INET;
						serverAddr.sin_port = htons(port);
						serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
						memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
						
						if(bind(filelisten, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
							cout<<"Cannot bind to file socket"<<endl;
							exit(1);
						}
						if(listen(filelisten, 1) != 0){
							cout<<"Error listening in file socket"<<endl;
							exit(1);
						}
						
						char buff2[20];
						sprintf(buff2, "CONN %d",port);
						send(cli, buff2, 20, 0);
						
						int pid1 = fork();
						if(pid1==0){ //child = file sending process
							close(cli);
							cout<<"file sending process"<<endl;
							addrsize=sizeof(cliAddr);
							clifile= accept(filelisten, (struct sockaddr *)&cliAddr, &addrsize);
							close(filelisten);
							memset(buff4, 0, 512);
							while((n = fread(buff4, 1, 512, fs)) > 0){
								//cout<<n<<endl;
								//cout<<buff4<<endl;
								send(clifile, buff4, n, 0);
								memset(buff4, 0, 512);
							}
							cout<<"entire file is sent"<<endl;
							fclose(fs);
							while(recv(clifile, buff4, 512, 0) >= 0); //wait till client closes the connection first
							close(clifile);
							cout<<"file sending process exiting"<<endl;
							exit(0);
							cout<<"error exiting of file process"<<endl;
						}
						else{ //parent
							close(filelisten);
							cout<<"parent waiting for new commands"<<endl;
							continue; //listens for more commands
						}
					}
				}
			}
			exit(0);
		}
		else{ //parent
			close(cli);
			cout<<"parent waiting for new connections"<<endl;
		}
	}
	
	
	return 0;
}

