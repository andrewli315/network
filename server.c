#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netdb.h>
#include<time.h>
int TCP_recv_file(int port)
{
	int socketfd, newsockfd, portno;
	socklen_t client_len;
	char buffer[1];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	char fname[256];
	char fsize[256];
	int file_size;
	int percent = 1;
	int recv_size = 0;
	
		
	socketfd = socket(AF_INET, SOCK_STREAM,0);
	if(socketfd<0){
		error("ERRPR opening a socket\n");
		return 1;
	}
	memset((char*)&serv_addr,0,sizeof(serv_addr));
	portno = port;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(socketfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR on binding\n");
		return 1;
	}
	listen(socketfd,5);
	client_len = sizeof(cli_addr);
	newsockfd = accept(socketfd,(struct sockaddr*)&cli_addr,&client_len);
	if(newsockfd < 0){
		error("ERROR on accepting\n");
		return 1;
	}
	printf("connect successfully\n");
	//read file name;
	memset(fname,0,256);
	n = read(newsockfd,fname,256);
	printf("%s\n",fname);
	if(n < 0){
		printf("ERROR in read file name\n");
		return 1;
	}
	//read file size;
	memset(fsize,0,256);
	n = read(newsockfd,fsize,256);
	if(n < 0){
		printf("ERROR in read file name\n");
		return 1;
	}
	file_size = atoi(fsize);
	printf("%d\n",file_size);
	FILE* fout = fopen(fname,"wb");
	if(fout == NULL){
		printf("failed to open file\n");
	}
	memset(buffer,0,1);
	sleep(1);
	while(1)
	{
		n = read(newsockfd,buffer,sizeof(buffer));
		if(n < 0){
			error("ERROR on reading network stream\n");
			break;
		}
		else if(n == 0){
			break;
		}
		recv_size++;
		fwrite(buffer,sizeof(char),n,fout);
		memset(buffer,0,1);
		sleep(0.5);
		
		//print the progress and time
		if(recv_size >= (percent*file_size/20) && percent*5 <= 100){
			
			time_t t = time(NULL);
			struct tm cur_time = *localtime(&t);
			printf("%d%% file has transmitted at %d:%d:%d in %d-%d-%d\n",percent*5,cur_time.tm_hour,cur_time.tm_min,
								cur_time.tm_sec,cur_time.tm_year+1900,cur_time.tm_mon+1,cur_time.tm_mday);
			percent++;					
		}
		else if(percent*5 > 100){
			break;	
		}
	}
	fclose(fout);
	close(newsockfd);
	close(socketfd);
	return 0;
	
}
int TCP_send_file(int port, char* fname)
{
	
}
int main(int argc, char* argv[])
{
	char *cmd[4] = {"tcp","udp","send","recv"};
	int portno;
	if(argc < 4){
		fprintf(stderr,"not enough parameter to execute the program\n");
		exit(1);
	}

	portno = atoi(argv[3]);
	printf("%s\n%s\n%s\n",argv[1],argv[2],argv[3]);
	if(strcmp(argv[1],cmd[0]) == 0)
	{
		if(strcmp(argv[2],cmd[3]) == 0)
		{
			printf("preparing for the server...\n");
			TCP_recv_file(portno);
		}

	}
	return 0;

}
