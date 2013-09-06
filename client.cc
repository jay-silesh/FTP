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
using namespace std;

#define PACKETSIZE 1500

void error(const char* msg)
{
	perror(msg);
	exit(0);

}

int main(int argc, char* argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	socklen_t servlen;
	char buffer[256];

	char filename[256];
	bzero(filename,256);

	if (argc < 4) {
	
		fprintf(stderr, "usage %s <source> <destination>  <port>\n", argv[0]);
		exit(0);

	}
	portno = atoi(argv[2]);
	ifstream file(argv[1], ifstream::binary);	

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
	bzero(buffer, PACKETSIZE);
        
	if(file)
	{
		file.read(buffer, PACKETSIZE);
		cout<<buffer;

	}
	
	file.close();
	n = sendto(sockfd, buffer, strlen(buffer), 0,
                        (struct sockaddr*) &serv_addr, servlen);
	cout<<n;
        /*if (n < 0)
        	error("ERROR on sendto");*/
	
	


	
	return 0;

}


