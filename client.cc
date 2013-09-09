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
int delivered_packets=1;


void create_listener();
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
	   	
	pid  = fork();
	if(pid < 0)
	{
		error("ERROR on fork");
		
	}
	if(pid!=0) {
		cout<<"\nCreated a child process....\n";
		create_listener();
		exit(0);
	}
	

	FILE *fd = fopen(argv[1],"r");
	if(fd == NULL)
	{
		error("ERROR IN OPENING A FILE");

	}
	int m=1,counter=0;
	while(1)
   	{

		char *buffer = (char *)calloc(sizeof(char)*(PACKETSIZE), 1);
		append_sequence_number(buffer, sequence_number);
		sequence_number++;
		m = fread(buffer +HEADERSIZE, 1, DATASIZE,fd);
		n = sendto(sockfd, buffer, PACKETSIZE, 0, (struct sockaddr*) &serv_addr, servlen);
		//cout<<"Sending the packet "<<sequence_number-1<<endl;
		
		counter = n+counter;
		
		if (n < 0)
        {	error("ERROR on sendto!"); 
    		exit(0);
   		}
        
     	if(m==0)
     	{
     		sequence_number = delivered_packets;
     		fseek(fd,sequence_number*DATASIZE,SEEK_SET);
     	} 
     	free(buffer); 	
        	
   	}
 
   	printf("\n\n%d bytes sent from client\n",counter);
   	close(sockfd);
   	fclose(fd);
	   
	return 0;
}


void create_listener()
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
		//sleep(5);
		

		if(start==0 && end==0)
		{
			cout<<"\nSIGKILL RECIEVED FROM THE SERVER...\n";
			exit(1);
		}
		else
			delivered_packets=start;
	
	}
	close(sockfd2);	
}


