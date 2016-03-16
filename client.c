#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netdb.h>
#include<time.h>
int getFileSize(char* fname)
{
	struct stat buf;
	int i = stat(fname,&buf);
	if(i == -1)
		return -1;
	return buf.st_size;
}
int TCP_trans_file(char* ip, int port, char* fname)
{
	int socketfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent* server;

	char temp;
	int count = 0;//to count 256 to record the buffer
	int file_size = 0;
	int trans_size = 0;//to record the transmiting prpgress
	int percent = 1;
	
	char buffer[1];
	char tmp[256];

	FILE* fin;
	fin = fopen(fname,"rb");
	
	if(fin == NULL){
		printf("ERROR in opening filfe\n");
		return 1;
	}

	file_size = getFileSize(fname);
	printf("file size : %d\n",file_size);
	
	//check whether the file opening is successful
	if(fin==NULL)
		error("ERROR in opening the file\n");
	
	portno = port;
	//use TCP protocol to connect to server
	socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if(socketfd < 0){
		error("ERROR in opening port\n");
		return 1;
	}
	server = gethostbyname(ip);
	
	//check whether the host exists
	if(server == NULL){
		fprintf(stderr,"no %s such host",ip);
		return 1;
	}
	//initial the server address with zero in bits
	memset((char*)&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)(server->h_addr),
			(char*)&serv_addr.sin_addr.s_addr,
				server->h_length);
	serv_addr.sin_port = htons(portno);
	//connect to server
	if(connect(socketfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
		printf("Error connecting\n");
		return 1;
	}
	//transmit file name
	n = write(socketfd,fname,strlen(fname));
	if(n < 0){
		printf("ERROR in writing to socket\n");
		return 1;
	}
	sleep(1);
	//transmit file size;
	char t[32];
	memset(t,0,sizeof(t));
	sprintf(t,"%d",file_size);
	n = write(socketfd,t,strlen(t));
	if(n < 0){
		error("ERROR on sending file size\n");
		return 1;
	}
	
	//transmit file;
	memset(buffer,0,1);
	while(feof(fin) !=  EOF){
		fread(buffer,sizeof(char),sizeof(buffer),fin);
		n = write(socketfd,buffer,sizeof(buffer));
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
	close(socketfd);
	return 0;	
}
int tcp_recv_file(char* ip,int port)
{
	int socketfd,portno,n;
	struct sockaddr_in serv_addr;
	struct hostent* server;
	char temp;
	char fsize[256];
	char fname[256];
	int file_size = 0;
	int recv_size = 0;//to record the recieving prpgress
	int percent = 1;
	char buffer[1];
	
	FILE* fout;
	
	portno = port;

	//use TCP protocol to connect to server
	socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if(socketfd < 0){
		error("ERROR in opening port\n");
		return 1;
	}
	
	server = gethostbyname(ip);
	
	//check whether the host exists
	if(server == NULL){
		fprintf(stderr,"no %s such host",ip);
		return 1;
	}

	//initial the server address with zero in bits
	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)(server->h_addr),
			(char*)&serv_addr.sin_addr.s_addr,
				server->h_length);
	serv_addr.sin_port = htons(portno);
	
	//connect to server
	if(connect(socketfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
		printf("Error connecting\n");
		return 1;
	}
	
	//recieve file name and open the file
	n = read(socketfd,fname,255);
	if(n < 0){
		printf("ERROR on read from socket\n");
		return 1;
	}
	printf("%s\n",fname);
	//open file after get file name
	fout = fopen(fname,"wr");
	if(fout == NULL)
	{
		printf("ERROR on opening file\n");
		return 1;
	}
	
	//recieve file size from server;
	n = read(socketfd,fsize,255);
	if(n < 0){
		printf("ERROR on read from socket\n");
		return 1;
	}
	file_size = atoi(fsize);
	printf("%d\n",file_size);
	//recieve file
	memset(buffer,0,sizeof(buffer));
	while(1)
	{
		n = read(socketfd,buffer,sizeof(buffer));
		if(n < 0){
			printf("ERROR on read from socket\n");
			return 1;
		}
		if(n == 0){
			
			break;
		}	
		fwrite(buffer,sizeof(char),n,fout);
		recv_size++;
		memset(buffer,0,sizeof(buffer));
		sleep(0.5);
		//print the progress and time
		if(recv_size >= (percent*file_size/20) && percent*5 <= 100){
			
			fflush(fout);
			time_t t = time(NULL);
			struct tm cur_time = *localtime(&t);
			printf("%d%% file has transmitted in %d:%d:%d in %d-%d-%d\n ",percent*5,cur_time.tm_hour,cur_time.tm_min,
											cur_time.tm_sec,cur_time.tm_year+1900,cur_time.tm_mon+1,cur_time.tm_mday);
			percent++;
		}
		else if(percent*5 > 100){
			
			break;
		}		
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

	if(argc<5){
		fprintf(stderr,"not enough parameter to execute the program\n");
		exit(0);
	}
	
	portno = atoi(argv[4]);
	//printf("%s\n%s\n%s\n%s\n",argv[1],argv[2],argv[3],argv[5]);
	if(strcmp(argv[1],cmd[0]) == 0)
	{
		if(strcmp(argv[2],cmd[2]) == 0)
		{
			printf("client is on its way\n");
			TCP_trans_file(argv[3],portno,argv[5]);
		}
		else if(strcmp(argv[2],cmd[3]) == 0)
		{
			printf("recv file\n");
			tcp_recv_file(argv[3],portno);
		}
		else
		{
			printf("wrong cmd : %s \n",argv[2]);
		}
	}	
	else if(strcmp(argv[1],cmd[1]) == 0)
	{
		if(strcmp(argv[2],cmd[2]) == 0)
		{
			
		}
		else if(strcmp(argv[2],cmd[3])== 0)
		{
			
		}
		else
		{
			printf("wrong cmd : %s \n",argv[2]);
		}
	}
	else
	{
		printf("wrong cmd : %s \n",argv[1]);
	}
	
	return 0;

}
