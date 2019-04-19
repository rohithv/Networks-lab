#include<iostream>
#include<cstdlib>
#include<cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

int main(){
	int cliSocket;
	struct sockaddr_in serverAddr;
	socklen_t addrSize;
	char buff4[512];
	
	
	cliSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(cliSocket < 0){
		cout<<"Socket creation failed\n";
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(20021);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
	
	addrSize = sizeof(serverAddr);
	if(connect(cliSocket, (struct sockaddr *)&serverAddr, addrSize) < 0){
		cout<<"Error connecting\n";
		exit(1);
	}
	
	cout<<"Connection established"<<endl;
	
	while(1){
		cout<<">";
		char buff1[50];
		fgets(buff1, 50, stdin);
		cout<<"Read: "<<buff1<<endl;
		char buff2[50];
		strncpy(buff2,buff1,50);
		char *token;
		token = strtok(buff2, " ");
		
		
		if(strcmp(buff1, "GOODBYE SERVER\n") == 0){ //goodbye
			cout<<"Sending: "<<buff1<<endl;
			send(cliSocket, buff1, 50, 0);
			char buff5[100];
			memset(buff5,0,100);
			recv(cliSocket, buff5, 50, 0);
			cout<<buff5<<endl;
			//Do something here
			exit(0);
		}
		else if(strcmp(token,"RETR")==0){ //RETR FILE
			FILE *fc;
			send(cliSocket, buff1, 50, 0);
			char buff3[100];
			recv(cliSocket, buff3, 50, 0); //CONN XXXXX
			int port = atoi(&buff3[5]);
			cout<<"port: "<<port<<endl;
			
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(port);
			serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
			
			int pid = fork();
			if(pid==0){ //child
				cout<<"Child for receiving"<<endl;
				close(cliSocket);
				int cli;
				cli = socket(PF_INET, SOCK_STREAM, 0);
				
				addrSize = sizeof(serverAddr);
				if(connect(cli, (struct sockaddr *)&serverAddr, addrSize) < 0){
					cout<<"Error connecting to new port"<<endl;
					close(cli);
					exit(1);
				}
				
				if(port == 20023){ //msg recv
					recv(cli, buff3, 100, 0);
					cout <<"Received: "<<buff3<<endl;
					close(cli);
					cout<<"message process exiting"<<endl;
					exit(0);
					cout<<"error in msg exit"<<endl;
				}
				else if(port == 20020){ //file transfer
					int n;
					
					FILE *fc;
					token = strtok(NULL, " \n");
					fc = fopen(token, "w+");
					cout<<"Writing to: "<<token<<endl;
					memset(buff4, 0, 512);
					while((n=recv(cli, buff4, 512, 0)) > 0){
						//cout<<n<<endl;
						//cout<<buff4<<endl;
						fwrite(buff4, 1, n, fc);
						//fwrite(buff4, 1, n, stdout);
						memset(buff4, 0, 512);
					}
					cout<<"file receiving complete"<<endl;
					close(cli);
					fclose(fc);
					cout<<"file process exiting"<<endl;
					exit(0);
					cout<<"error in file exit"<<endl;
				}
				exit(0);
			}
			else{//parent
				cout<<"parent waiting"<<endl;
				continue;
			}
			
		}
	}
	
	return 0;
}
