#include<iostream>
#include<cstdlib>
#include<cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080

using namespace std;

int main(){
	int cliSocket;
	struct sockaddr_in serverAddr;
	socklen_t addrSize;
	char buff[100];
	uint32_t file_size, nw_file_size;
	
	cliSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(cliSocket < 0){
		cout<<"Socket creation failed\n";
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
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
		char buff1[100];
		memset(buff1, 0, 100);
		fgets(buff1, 100, stdin);
		cout<<"Read: "<<buff1;
		
		send(cliSocket, buff1, 100, 0);
		
		char *method, *file, *http, *token;
		method = strtok(buff1, " \n");
		file = strtok(NULL, " \n");
		http = strtok(NULL, " \n");
		token = strtok(NULL, " \n");
		
		if(strcmp(method,"GET")==0){
			//cout<<"cliget"<<endl;
			recv(cliSocket, buff, 100, 0);
			cout<<buff<<endl;
			if(strcmp(buff,"HTTP/1.1 200")!=0){
				continue;
			}
			else{//success
				//read filesize
				recv(cliSocket, &file_size, sizeof(file_size), 0);
				file_size = ntohl(file_size);
				//receive the data and write to a stdout
				int num_recv = 0;
				while(num_recv < file_size){
					int n = recv(cliSocket, buff, 100, 0);
					if(n<=0){
						printf("server closed the connection\n");
						break;
					}
					fwrite(buff, 1, n, stdout);
					num_recv += n;
				}
			}
		}
		
		else if(strcmp(method,"PUT")==0){
			//cout<<"cliput"<<endl;
			
			FILE *fp = fopen(file+1, "r");
			if(fp==NULL){
				cout<<"File not found"<<endl;
				file_size=0;
				nw_file_size = htonl(file_size);
				send(cliSocket, &nw_file_size, sizeof(nw_file_size), 0);
				continue;
			}
			fseek(fp,0L,SEEK_END);
			file_size= ftell(fp); //Assumes file size is less than 2^32 i.e., 4GB.
			nw_file_size = htonl(file_size);
			fseek(fp,0L,SEEK_SET);
			
			//Sending file size first
			send(cliSocket, &nw_file_size, sizeof(nw_file_size), 0);
			
			//sending data of file in blocks of 100.
			int num_sent = 0;
			while(num_sent < file_size){
				int n = fread(buff, 1, 100, fp);
				send(cliSocket, buff, n, 0);
				num_sent+=n;
			}
			fclose(fp);
			
			recv(cliSocket, buff, 100, 0);
			cout<<buff<<endl;
		}
		
		else{
			memset(buff, 0, 100);
			recv(cliSocket, buff, 100, 0);
			cout<<buff<<endl;
		}
	}
	return 0;
}
