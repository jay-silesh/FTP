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
using namespace std;

#define PACKETSIZE 1400

static uint64_t sequence_number = 0;

void append_sequence_number(char *packet)
{
	char number[sizeof(uint64_t)];
	stringstream ss;
	ss << sequence_number++;
	ss>>number;
	strncpy(packet + PACKETSIZE - sizeof(uint64_t) , number, sizeof(uint64_t));
	


} 

void error(const char* msg)
{
	perror(msg);
	exit(0);

}

int main(int argc, char* argv[])
{
	int sockfd, portno, n,data_read;
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
	bcopy((char*)server->h_addr,
	      (char*)&serv_addr.sin_addr.s_addr,
	      server->h_length);
	serv_addr.sin_port = htons(portno);
	servlen = sizeof(serv_addr);
	//bzero(buffer, PACKETSIZE);
       printf("hello\n"); 
	FILE *fd = fopen(argv[1],"r");
	if(fd == NULL)
	{
		error("ERROR IN OPENING A FILE");

	}
	int m=1,counter=0;
	while(m!=0)
   	{
		buffer = (char *)calloc(sizeof(char)*PACKETSIZE,1);
		append_sequence_number(buffer);
		m = fread(buffer, 1, PACKETSIZE - sizeof(uint64_t),fd);
		
		n = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*) &serv_addr, servlen);
		counter = n+counter;
		if (n < 0)
        	error("ERROR on sendto"); 
        	free(buffer);
		//bzero(buffer, PACKETSIZE);
        	
        	
   	}
   	n = sendto(sockfd, buffer, 0, 0, (struct sockaddr*) &serv_addr, servlen);
   	printf("%d bytes sent from client\n",counter);
   	close(sockfd);
   fclose(fd);
	   

	
	
	
     


	return 0;

}


