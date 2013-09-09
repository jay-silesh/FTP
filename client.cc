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

#define SEND_TIMES 4

int ack_sleep_time=2000;

int sequence_number = 1;
int delivered_packets=1;

FILE *fd;

int sockfd;
struct sockaddr_in serv_addr;
struct hostent *server;
socklen_t servlen;


void *create_listener(void * arg1);
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


int send_packet(int sq_no)
{
	char *buffer = (char *)calloc(sizeof(char)*(PACKETSIZE), 1);
	append_sequence_number(buffer, sequence_number);
	fseek(fd,sq_no*DATASIZE,SEEK_SET);
	int m = fread(buffer +HEADERSIZE, 1, DATASIZE,fd);
	int n = sendto(sockfd, buffer, PACKETSIZE, 0, (struct sockaddr*) &serv_addr, servlen);
	if (n < 0)
    {	error("ERROR on sendto!"); 
    	exit(0);
   	}
   	cout<<"Sending the packet "<<sq_no<<endl;
   	free(buffer); 
   	return m;   
}


int main(int argc, char* argv[])
{
	int portno, n,data_read;
	pid_t pid;
	
	char filename[256];
	bzero(filename,256);

	if (argc < 4) {
		fprintf(stderr, "usage %s <source> <destination>  <port>\n", argv[0]);
		exit(0);
	}

	pthread_t listener_thread;
	int iret1 = pthread_create( &listener_thread, NULL,create_listener,NULL);

	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[2]);
	portno = atoi(argv[3]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
		
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,
		(char*)&serv_addr.sin_addr.s_addr,
		 server->h_length);
	serv_addr.sin_port = htons(portno);
	servlen = sizeof(serv_addr);
	   	
	fd = fopen(argv[1],"r");
	if(fd == NULL)
	{
		error("ERROR IN OPENING A FILE");

	}
	int m=1;
	while(1)
   	{
		m=send_packet(sequence_number);
		sequence_number++;		
		     
     	if(m==0)
     	{
     		sequence_number = delivered_packets;
     		fseek(fd,sequence_number*DATASIZE,SEEK_SET);
     	} 
     		
	}
 
   	close(sockfd);
   	fclose(fd);
	   
	return 0;
}


void *create_listener(void * arg1)
{
	cout<<"\n\nCreated the listener......\n\n\n";

	int sockfd2,newsockfd2;
	socklen_t serverlen2;

	struct sockaddr_in serv_addr2, cli_addr2;
	sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd2 < 0) 
		error("ERROR opening socket");

	bzero((char *) &cli_addr2, sizeof(cli_addr2));
	cli_addr2.sin_family = AF_INET;
	cli_addr2.sin_addr.s_addr = INADDR_ANY;
	cli_addr2.sin_port = htons(PORTNUMBER);

	if (bind(sockfd2, (struct sockaddr *) &cli_addr2,sizeof(cli_addr2)) < 0) 
	  error("ERROR on binding");
	
	listen(sockfd2,5);
	serverlen2 = sizeof(serv_addr2);

	newsockfd2 = accept(sockfd2,(struct sockaddr *) &serv_addr2,&serverlen2);
	if (newsockfd2 < 0) 
		error("ERROR on accept");

	while (1)
	{
		int start,end;

		cout<<"\nAccepted connection....";		
		char *buffer2 = (char *)calloc(sizeof(char)*10, 1);		
		cout<<"\nWaiting to read...\n";
		int n = read(newsockfd2,buffer2,sizeof(int)*2);
		if (n < 0) 
			error("ERROR reading from socket");

		memcpy(&start, buffer2 + 0, 4);
		memcpy(&end, buffer2 + 4, 4);
		start = ntohl(start);
		end = ntohl(end);
		
		cout<<"\nReading data.....start"<<start<<" end"<<end<<endl;
		if(start==0 && end==0)
		{
			cout<<"\nSIGKILL RECIEVED FROM THE SERVER...\n";
			exit(1);
		}
		else
			delivered_packets=start;
		
		for(int temp_i=1;temp_i<=SEND_TIMES;temp_i++)
			send_packet(delivered_packets);

		free(buffer2);
	
	}
	close(sockfd2);	
}


