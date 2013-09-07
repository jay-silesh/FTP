#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <iostream>
#include <sstream>
using namespace std;

#define PACKETSIZE 1400

void error(const char *msg)
{
	perror(msg);
	exit(1);

}

uint64_t get_sequence_number(char *packet)
{
	char number[sizeof(uint64_t) +1];
	uint64_t seq;
	stringstream ss;

	bzero(number, sizeof(uint64_t) +1);
	strncpy(number,packet + PACKETSIZE - sizeof(uint64_t),sizeof(uint64_t));
	ss<<number;
	ss>>seq;

	//return the seq number and then make the last 8 bytes as 0 so 
	//the server will have pure data packet.
	bzero(packet + PACKETSIZE - sizeof(uint64_t), sizeof(uint64_t));
	return seq;	

	

}

int main(int argc, char *argv[1])
{
	int sockfd, portno;
	pid_t pid;
	uint64_t sequence_number;
	socklen_t clilen;
	char buffer[PACKETSIZE];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	signal(SIGCHLD,SIG_IGN);
	if (argc < 2) {

		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {	
		
		error("ERROR opening socket");

	}
	bzero((char*)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	
		error("ERROR on binding");
	clilen = sizeof(cli_addr);
	FILE *fd = fopen("output.txt","a+");
	if(fd==NULL)
	{
		printf("Error while opening file\n");
		exit(1);
	}
		//	printf("packet size %ld\n",sizeof(buffer));
	int counter=0;
	while(1) 
	{ 

		
		bzero(buffer,PACKETSIZE);
		n = recvfrom(sockfd, buffer, sizeof(buffer), 0,(struct sockaddr*) &cli_addr, &clilen);
		if (n < 0)
		error("ERROR on recvfrom");
		
		
		if(n==0)
		{
				break;
		}
		sequence_number =  get_sequence_number(buffer);	
		fwrite(buffer,sizeof(char),strlen(buffer),fd);
		counter = counter + n;	

	}
	printf("%d bytes received\n",counter);
	close(sockfd);
	fclose(fd);
	return 0;
}






