#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netinet/in.h>
#include<netdb.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
int getFileSize(char* fname)
{
	struct stat buf;
	int i = stat(fname,&buf);
	if(i == -1)
		return -1;
	return buf.st_size;
}
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
			
			fflush(fout);
			time_t t = time(NULL);
			struct tm cur_time = *localtime(&t);
			printf("%d%% file has transmitted at %d:%d:%d in %d-%d-%d\n",percent*5,
						cur_time.tm_hour,cur_time.tm_min,cur_time.tm_sec,
						cur_time.tm_year+1900,cur_time.tm_mon+1,cur_time.tm_mday);
			
			percent++;					
		}
		else if(percent*5 > 100){
			break;	
		}
	}
	fflush(fout);
	fclose(fout);
	close(newsockfd);
	close(socketfd);
	return 0;
	
}
int TCP_send_file(int port, char* fname)
{
	int socketfd, newsockfd, portno;
	socklen_t client_len;
	char buffer[1];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int file_size;
	int percent = 1;
	int trans_size = 0;
	
	FILE* fin = fopen(fname,"rb");
	if(!fin)
	{
		error("ERROR on opening file\n");
		return 1;
	}
	
	file_size = getFileSize(fname);
	
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
	
	//transmit file name
	n = write(newsockfd,fname,strlen(fname));
	if(n < 0){
		printf("ERROR in writing to socket\n");
		return 1;
	}
	sleep(1);
	
	//transmit file size;
	char t[32];
	memset(t,0,sizeof(t));
	printf("%d  %s\n",file_size,t);
	sprintf(t,"%d",file_size);
	n = write(newsockfd,t,strlen(t));
	if(n < 0){
		error("ERROR on transmit file name\n");
		return 1;
	}
	
	memset(buffer,0,sizeof(buffer));
	while(feof(fin) != EOF)
	{
		fread(buffer,sizeof(char),sizeof(buffer),fin);
		n = write(newsockfd,buffer,sizeof(buffer));
		if(n < 0){
			printf("ERROR in writing to socket\n");
			break;
		}
		trans_size++;
		memset(buffer,0,1);
		//print the progress and time
		if(trans_size >= (percent*file_size/20) && percent*5 <= 100 ){	
			time_t t = time(NULL);
			struct tm cur_time = *localtime(&t);
			printf("%d%% file has transmitted at %d:%d:%d in %d-%d-%d\n ",percent*5,cur_time.tm_hour,cur_time.tm_min,
											cur_time.tm_sec,cur_time.tm_year+1900,cur_time.tm_mon+1,cur_time.tm_mday);
			percent++;
		}
		else if(percent*5 > 100){
			break;
		}
	}
	
	fclose(fin);
	close(newsockfd);
	close(socketfd);
	return 0;
	
}
int UDP_recv_file(int port)
{
	int socketfd,n;
	int  portno,length;
	struct sockaddr_in serv_addr,cli_addr;
	int num = 0;
	char ACK = 6;
	char SYN = 22;
	char cmd;
	int file_size;
	int percent = 1;
	int recv_size;
	char fname[256];
	char fsize[256];
	char buffer[256];
	char temp[300];
	char seq_num[4];
	int i,s_num;
	FILE* fout;

	socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (n < 0) {
        error("ERROR on create an udp socket\n");
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
	length = sizeof(cli_addr);
	
	memset(fname,0,sizeof(fname));
	n = recvfrom(socketfd,fname,sizeof(fname),0,(struct sockaddr*)&cli_addr,&length);
	printf("File name      : %s\n",fname);
	if(n < 0)
	{
		printf("ERROR on recieving file name\n");
		return 1;
	}
	n = sendto(socketfd,fname,sizeof(fname),0,(struct sockaddr*)&cli_addr,length);
	if(n<0)
	{
		printf("ERROR on writing\n");
		return 1;
	}
	memset(fsize,0,sizeof(fsize));
	n = recvfrom(socketfd,fsize,sizeof(fsize),0,(struct sockaddr*)&cli_addr,&length);
	if(n < 0)
	{
		printf("ERROR on recieving file size\n");
		return 1;
	}
	n = sendto(socketfd,fname,sizeof(fname),0,(struct sockaddr*)&cli_addr,length);
    if(n<0)
    {
        printf("ERROR on writing\n");
        return 1;                                                               
    }

	printf("%s\n",fsize);

	
	fout = fopen(fname,"wb");

	while(1){
		n = recvfrom(socketfd,buffer,sizeof(buffer),0,(struct sockaddr*)&cli_addr,&length);
		printf("%s\n",buffer);
		if(n < 0){
			printf("error on recieving\n");
			return 1;
		}
		else if(buffer == NULL)
			break;
		else if(n ==0)
		{
			sendto(socketfd,buffer,sizeof(buffer),0,(struct sockaddr*)&cli_addr,length);
		}
		else if(strcmp(buffer,"end")==0)
			break;

		fwrite(buffer,sizeof(char),n,fout);
		sendto(socketfd,buffer,sizeof(buffer),0,(struct sockaddr*)&cli_addr,length);
		memset(buffer,0,256);
		memset(temp,0,300);
	}
	fflush(fout);
	fclose(fout);
	close(socketfd);
	return 0;
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
		else if(strcmp(argv[2],cmd[2]) == 0)
		{
			printf("preparing for the server...\n");
			TCP_send_file(portno,argv[4]);
		}
	}
	else if(strcmp(argv[1],cmd[1]) == 0)
	{
		if(strcmp(argv[2],cmd[3]) == 0)
		{
			UDP_recv_file(portno);
		}
	}
	return 0;
}
