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
#include <pthread.h>
#include "Packet.h"

using namespace std;

#define PACKETSIZE 1400
#define DATASIZE PACKETSIZE-sizeof(int)
#define HAEDERSIZE sizeof(int)

char addr_str[INET_ADDRSTRLEN+1];

int sockfd, portno;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;


map<int, char*> recieved_map;
map<int, char*>::iterator it;
map<int, char*>::reverse_iterator rit;
map<int, char*> storing_map;


int last_packet_received = 0; //last inorder received
int sequence_number;


void error(const char *msg)
{
	perror(msg);
	exit(1);

}



void *check_map(void* flag_retran)
{

	bool *flag_retrans=(bool*)flag_retran;
	cout<<"\n\nChecking map and sending retransmission packets..........\n\n";
	for(it = recieved_map.find(last_packet_received);it!=recieved_map.end();)
	{	
		it++;
		if(it->first==last_packet_received+1)
		{
			last_packet_received++;
		}
		else
		{
			if(*flag_retrans)
			{
				int temp=it->first;
				char *buffer = (char *)calloc(sizeof(char)*10, 1);
				int x = htonl(sequence_number);	
				memcpy(buffer, &last_packet_received, sizeof(int));
				memcpy(buffer + 4, &temp, sizeof(int));		
				sendto(sockfd,buffer,sizeof(int)*2,0,(struct sockaddr *) &cli_addr,sizeof(sockaddr_in));
				break;
			}
			else
				break;

		}
	}

}






int main(int argc, char *argv[1])
{
	char buffer[PACKETSIZE];
	pthread_t check_map_thread,update_map;
	bool first_run=true;

	bool true_flag=true,false_flag=false;
	

	
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
	
	int counter=0;
	int check_counter=0;
	bool check_flag=false;
	while(1) 
	{ 

		char *data;
		bzero(buffer,PACKETSIZE);
		n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
		
		if(first_run)
		{
			inet_ntop(AF_INET,&(cli_addr.sin_addr),addr_str,INET_ADDRSTRLEN);
			first_run=false;
		}

		if (n < 0)
		error("ERROR on recvfrom");		
		if(n==0)
			break;
		
		//Extracting the data from the packet
		sequence_number =  get_sequence_number(buffer);
		data=get_data(buffer);
		
		it = recieved_map.find(sequence_number);
		if ( it==recieved_map.end()) {
		
			recieved_map.insert(pair<int,char*>(sequence_number,data));			
			if(sequence_number == (last_packet_received + 1) )
			{

				last_packet_received = sequence_number;
				if(check_flag)
				{
					check_flag=false;
					check_map(&true_flag);
					//iret2 = pthread_create( &update_map_thread, NULL, update_map, NULL);	
				}
				
			}
			else
			{
				check_flag=true;
				int iret1 = pthread_create( &check_map_thread, NULL,check_map, (void*)&false_flag);
			}			
			counter = counter + n;	
		
		}
		//else do not write ie: duplicata packet.drop it
		
		
	}

	for(it=recieved_map.begin();it!=recieved_map.end();it++)
		cout<<"\n\n**************\n\n"<<it->first<<"-->"<<it->second;


	close(sockfd);
	fclose(fd);
	return 0;
}








