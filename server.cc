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
#include <map>
using namespace std;

#define PACKETSIZE 1400

void error(const char *msg)
{
	perror(msg);
	exit(1);

}

int get_sequence_number(char *packet)
{
	//char number[sizeof(int) +1];
	int seq, x;
	
	//stringstream ss;

	//bzero(number, sizeof(int) +1);
	//strncpy(number,packet,sizeof(int));
	//ss<<number;
	//ss>>seq;
	memcpy(&seq, packet, sizeof(int));
	x = ntohl(seq);
	return x;	

	

}

int main(int argc, char *argv[1])
{
	int sockfd, portno;
	pid_t pid;
	int last_packet_received = 0; //last inorder received
	map<int, bool> sequence_map;
	map<int, bool>::iterator it;
	int sequence_number;
	socklen_t clilen;
	char buffer[PACKETSIZE];
	char data[PACKETSIZE];
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
		bzero(data,PACKETSIZE);
		n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
		if (n < 0)
		error("ERROR on recvfrom");
		
		
		if(n==0)
		{
				break;
		}
		sequence_number =  get_sequence_number(buffer);
		memcpy(data, buffer+sizeof(int), PACKETSIZE- sizeof(int));	
		it = sequence_map.find(sequence_number);
		if ( it->second == false ) {
			it->second = true;
			if (sequence_number == (last_packet_received + 1) )
			{
				last_packet_received = sequence_number;
			}
			fwrite(data,sizeof(char),PACKETSIZE,fd);
			counter = counter + n;	

		}
		//else do not write ie: duplicata packet.drop it

	}
	printf("%d bytes received\n",counter);
	close(sockfd);
	fclose(fd);
	return 0;
}






