#include<iostream>
#include<cstdlib>
#include<cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080

using namespace std;
int bad(char *method, char *file, char *http, char *token);
int check404(char *file, int cli);
int check403(char *file, int cli);

int main(){
	int welcomesocket, cli, n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage cliAddr;
	socklen_t addrsize;
	uint32_t file_size, nw_file_size;
	char buff[100];
	char index[20];
	FILE *fp;
	
	welcomesocket = socket(PF_INET, SOCK_STREAM, 0);
	if(welcomesocket < 0){
		cout<<"Socket creation failed"<<endl;
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
	
	if(bind(welcomesocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
		cout<<"Cannot bind"<<endl;
		exit(1);
	}
	
	if(listen(welcomesocket, 10) != 0){
		cout<<"Error listening"<<endl;
		exit(1);
	}
	cout<<"Listening for connections"<<endl;
	
	while(1){
		addrsize=sizeof(cliAddr);
		cli= accept(welcomesocket, (struct sockaddr *)&cliAddr, &addrsize);
		cout<<"Received a connection"<<endl;
		
		int pid = fork();
		if(pid==0){ //Child process
			close(welcomesocket);
			while((n=recv(cli, buff, 100, 0)) > 0){ 
				cout<<buff;
				char *method, *file, *http, *token;
				char buff2[100];
				strncpy(buff2, buff, 100);
				method = strtok(buff2, " \n");
				file = strtok(NULL, " \n");
				http = strtok(NULL, " \n");
				token = strtok(NULL, " \n");
				
				//checking for BAD REQUESTS
				if(bad(method, file, http, token)){
					strcpy(buff, "HTTP/1.1 400");
					send(cli, buff, 100, 0);
					cout<<buff<<endl;
					continue;
				}
				// if file is "/" treat it as index.html
				if(strcmp(file,"/")==0){
					cout<<"treating file as index.html"<<endl;
					strcpy(index, "/index.html");
					file = index;
				}
				
				if(strcmp(method,"GET")==0){
					cout<<"get "<<file<<endl;
					if(check404(file, cli) || check403(file, cli)){
						continue;
					}
					else{ //send file -> 200
						fp = fopen(file+1, "r");
						
						memset(buff, 0, 100);
						strcpy(buff, "HTTP/1.1 200");
						send(cli, buff, 100, 0);
						
						fseek(fp,0L,SEEK_END);
						file_size= ftell(fp); //Assumes file size is less than 2^32 i.e., 4GB.
						nw_file_size = htonl(file_size);
						fseek(fp,0L,SEEK_SET);
						
						//Sending file size first
						send(cli, &nw_file_size, sizeof(nw_file_size), 0);
						
						//sending data of file in blocks of 100.
						int num_sent = 0;
						while(num_sent < file_size){
							n = fread(buff, 1, 100, fp);
							send(cli, buff, n, 0);
							num_sent+=n;
						}
						fclose(fp);
					}
				}
				else if(strcmp(method, "HEAD")==0){
					cout<<"head "<<file<<endl;
					if(check404(file, cli) || check403(file, cli)){
						continue;
					}
					else{ //send header -> 200
						char time[50];
						struct stat attr;
						stat(file+1,&attr);
						strftime(time,50,"Last Modified: %d/%m/%y---%H:%M:%S",localtime(&(attr.st_ctime)));
						memset(buff, 0, 100);
						strcpy(buff, "HTTP/1.1 200\n");
						strcat(buff,time);
						send(cli, buff, 100, 0);
					}
				}
				else if(strcmp(method, "PUT")==0){
					cout<<"put "<<file<<endl;
					if(check403(file, cli)){
						//discard the file without writing
						recv(cli, &file_size, sizeof(file_size), 0);
						file_size = ntohl(file_size);
						int num_recv = 0;
						while(num_recv < file_size){
							n = recv(cli, buff, 100, 0);
							if(n<=0){
								printf("client closed the connection\n");
								break;
							}
							num_recv += n;
						}
						continue;
					}
					else{ //receive file -> 201
						
						recv(cli, &file_size, sizeof(file_size), 0);
						file_size = ntohl(file_size);
						if(file_size==0){
							continue;
						}
						int n = strlen(file);
						while(n>0 && file[n]!='/'){
							n--;
						}
						cout<<"writing to: "<<file+n+1;
						fp=fopen(file+n+1, "w+");
						int num_recv = 0;
						while(num_recv < file_size){
							n = recv(cli, buff, 100, 0);
							if(n<=0){
								printf("client closed the connection\n");
								break;
							}
							fwrite(buff, 1, n, fp);
							num_recv += n;
						}
						fclose(fp);
						memset(buff, 0, 100);
						strcpy(buff, "HTTP/1.1 201");
						send(cli, buff, 100, 0);
						
					}
				}
				
				else if(strcmp(method, "DELETE")==0){
					cout<<"put "<<file<<endl;
					if(check404(file, cli) || check403(file, cli)){
						continue;
					}
					else{ //delete file -> 200
						remove(file+1);
						memset(buff, 0, 100);
						strcpy(buff, "HTTP/1.1 200");
						send(cli, buff, 100, 0);
					}
				}
				
				else{
					strcpy(buff, "HTTP/1.1 405");
					send(cli, buff, 100, 0);
					cout<<buff<<endl;
				}
			}
			exit(0);
		}
		else{ //parent
			close(cli);
			continue;
		}
		
	}
	
	return 0;
}

int bad(char *method, char *file, char *http, char *token){
	if(method==NULL || file ==NULL || http==NULL || token!=NULL)
		return 1;
	if(strcmp(http, "HTTP/1.1")!=0)
		return 1;
	if(file[0]!='/')
		return 1;
	return 0;
}

int check404(char *file, int cli){
	char buff[100];
	FILE *fp;
	if((fp=fopen(file+1,"r")) == NULL){
		strcpy(buff, "HTTP/1.1 404");
		send(cli, buff, 100, 0);
		return 1;
	}
	else{
		fclose(fp);
		return 0;
	}
}

int check403(char *file, int cli){
	char buff[100];
	int n = strlen(file);
	while(n>0 && file[n]!='/'){
		n--;
	}
	if((file[n+1]=='1') && (file[n+2]=='1')){
		strcpy(buff, "HTTP/1.1 403");
		send(cli, buff, 100, 0);
		return 1;
	}
	return 0;
}
