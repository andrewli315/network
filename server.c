#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>

int main()
{
	int sockfd, newsockfd, portno;
	socklen_t client_len;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if(argc < 2){
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_	INET, SOCK_STREAM,0);
	if(sockfd<0)
		error("ERRPR opening a socket\n");
	memset((char*)&serv_addr,sizeof(serv_addr);
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr INADDR_ANY;
	
	
}