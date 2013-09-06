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
using namespace std;

#define PACKETSIZE 1500

void error(const char *msg)
{
	perror(msg);
	exit(1);

}

int main(int argc, char* argv[])
{
	int sockfd, portno;
	pid_t pid;
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
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	clilen = sizeof(cli_addr);
	while(1) { 

		bzero(buffer,PACKETSIZE);
		n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                	(struct sockaddr*) &cli_addr, &clilen);
		if (n < 0)
			error("ERROR on recvfrom");

		cout<<buffer;		
			

	}
	close(sockfd);
	return 0;
}





