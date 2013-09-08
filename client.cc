#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Packet.h"
#include <queue>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <signal.h>
#include <semaphore.h>
using namespace std;


static int sequence_number = 1;
sem_t transmit;

void retransmitter(int, int, int, struct sockaddr_in,char []);
void create_listener(int,struct sockaddr_in,char []);
void append_sequence_number(char *packet,int sequence_no)
{
	int x = htonl(sequence_no);	
	memcpy(packet + 0, &x, HEADERSIZE);

} 

void error(const char* msg)
{
	perror(msg);
	exit(0);

}

int main(int argc, char* argv[])
{
	int sockfd, portno, n,data_read;
	pid_t pid;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	//char buffer[256];
	socklen_t servlen;
	//char buffer[PACKETSIZE];
	char *buffer;
	char filename[256];
	bzero(filename,256);

	if (argc < 4) {
	
		fprintf(stderr, "usage %s <source> <destination>  <port>\n", argv[0]);
		exit(0);

	}



	portno = atoi(argv[3]);
//	ifstream file(argv[1], ifstream::binary);	
	

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[2]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
		
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	servlen = sizeof(serv_addr);
	   	
	if( sem_init(&transmit,1,1) < 0)
    	{
    	  error("semaphore initilization");
     	   exit(0);
    	}
	pid  = fork();
	if(pid < 0)
	{
	
		error("ERROR on fork");
		
	}
	if(pid == 0) {

		close(sockfd);
		create_listener(sockfd, serv_addr, filename);
		exit(0);
	}
	 
	FILE *fd = fopen(argv[1],"r");
	if(fd == NULL)
	{
		error("ERROR IN OPENING A FILE");

	}
	int m=1,counter=0;
	while(m!=0)
   	{

		buffer = (char *)calloc(sizeof(char)*(PACKETSIZE), 1);
		append_sequence_number(buffer, sequence_number);
		sequence_number++;
		m = fread(buffer +HEADERSIZE, 1, DATASIZE,fd);
		
		sem_wait(&transmit);
		n = sendto(sockfd, buffer, PACKETSIZE, 0, (struct sockaddr*) &serv_addr, servlen);
		cout<<"Sending the packet "<<sequence_number-1<<endl;
		sem_post(&transmit);

		//usleep(500);
		counter = n+counter;
		
		if (n < 0)
        	error("ERROR on sendto"); 
        	free(buffer);
		//bzero(buffer, PACKETSIZE);
        	
        	
   	}
   //	n = sendto(sockfd, buffer, 0, 0, (struct sockaddr*) &serv_addr, servlen);
   	printf("\n\n%d bytes sent from client\n",counter);
   	close(sockfd);
   	fclose(fd);
	   

	
	
	
     


	return 0;

}

void create_listener(int original_sockfd, struct sockaddr_in original_serv_addr,char filename[256])
{
	
	int sockfd, portno, n;
	socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
	signal(SIGCHLD,SIG_IGN);
	char buffer[10];
	pid_t pid;
	int start,end;	

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {

                error("ERROR opening socket");

        }
        bzero((char*)&serv_addr, sizeof(serv_addr));
        portno = PORTNUMBER;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
                error("ERROR on binding");
        clilen = sizeof(cli_addr);
	while (1) {
	
		bzero(buffer,10);
		n = recvfrom(sockfd, buffer, sizeof(int)*2, 0,(struct sockaddr*) &cli_addr, &clilen);
        	if (n < 0)
               	error("ERROR on recvfrom");


	
	
		memcpy(&start, buffer + 0, 4);
		memcpy(&end, buffer + 4, 4);
		cout<<"start"<<start<<" end"<<end<<endl;
		pid = fork();
		if(pid < 0) 
			error("Error on fork in listener");
		if(pid == 0)
		{

			
			retransmitter(start,end, original_sockfd,original_serv_addr, filename);
			close(sockfd);
			//close(original_sockfd);
			exit(0);

		}	

	}
}


void retransmitter(int start, int end, int sockfd, struct sockaddr_in serv_addr,char filename[256])
{
	cout<<filename;
	int counter = start;
	char *buffer;
	int m,n;
	socklen_t servlen;
	FILE *fp = fopen(filename, "r");
	if(fp == NULL)
    {
		error("ERROR IN OPENING A FILE");
	}
	sem_wait(&transmit);
	while(counter < start)
	{
		buffer = (char *)calloc(sizeof(char)*(PACKETSIZE), 1);
        append_sequence_number(buffer, counter);
		fseek(fp, (counter-1)*DATASIZE,SEEK_SET);
        m = fread(buffer + HEADERSIZE, 1, DATASIZE,fp);
        n = sendto(sockfd, buffer, PACKETSIZE, 0, (struct sockaddr*) &serv_addr, servlen);
		counter++;
	}
	sem_post(&transmit);

	exit(0);
}
