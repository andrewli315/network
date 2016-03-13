#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netdb.h>
#include<time.h>
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

	FILE *fin = fopen(fname,"r");
	if(NULL == fin){
		printf("ERROR in opening filfe\n");
		return 1;
	}
	while((temp = fgetc(fin))!=EOF){
		file_size++;
	}
	
	//reopen the file
	fclose(fin);
	fopen(fname,"r");

	char buffer[256];
	
	//check whether the file opening is successful
	if(fin==NULL)
		error("ERROR in opening the file\n");
	
	portno = port;

	//use TCP protocol to transmit file
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
	
	//initial the buffer
	bzero(buffer,256);
	while((temp = fgetc(fin)!= EOF)){
		if(count == 255){
			n = write(socketfd,buffer,strlen(buffer));
			if(n < 0){
				printf("ERROR in writing to socket\n");
				break;
			}
			trans_size++;
			bzero(buffer,256);
			count = 0;
		}
		else{
			trans_size++;
			buffer[count++] = temp;
		}
		
		//print the progress and time
		if(trans_size == (percent*file_size/20)){
			
			time_t t = time(NULL);
			struct tm cur_time = *localtime(&t);
			printf("%d%% file has transmitted in %d %d %d %d-%d-%d\n ",percent*5,cur_time.tm_hour,cur_time.tm_min,
											cur_time.tm_sec,cur_time.tm_year+1900,cur_time.tm_mon+1,cur_time.tm_mday);
		}
	}
	n =  write(socketfd,buffer,strlen(buffer));	
	if(n < 0)
		return 1;
	bzero(buffer,256);
	n = read(socketfd,buffer,strlen(buffer));
	if(n < 0)
		error("ERROR in reading from socket");
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
	
	portno = atoi(argv[2]);
	if(strcmp(argv[1],cmd[0]))
	{
		if(strcmp(argv[2],cmd[2]))
		{
			TCP_trans_file(argv[3],portno,argv[5]);
		}
		else if(strcmp(argv[2],cmd[3]))
		{
			
		}
		else
		{
			printf("wrong cmd : %s \n",argv[2]);
		}
	}	
	else if(strcmp(argv[1],cmd[1]))
	{
		if(strcmp(argv[2],cmd[2]))
		{
			
		}
		else if(strcmp(argv[2],cmd[3]))
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
