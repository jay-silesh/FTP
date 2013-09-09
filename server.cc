#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <queue>
#include <fstream>
#include <netdb.h>
#include <sstream>
#include <signal.h>
#include <sstream>
#include <map>
#include <pthread.h>
#include "Packet.h"

using namespace std;

int ack_sleep_time=2000;


char addr_str[INET_ADDRSTRLEN+1];

char * client_name;

int sockfd, portno;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

int sockfd_s;
struct sockaddr_in serv_addr_s;
struct hostent *server_s;
socklen_t servlen_s;

bool retrans_flag=true;

map<int, char*> recieved_map;
map<int, char*>::iterator it;

static int last_packet_received = 0; //last inorder received
static int last_out_of_order_packet_received = 0; //last inorder received
static int last_packet_number=-1;
static int sequence_number;


void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void print_data();

bool check_all_packets_recieved()
{
	if(recieved_map.size()==last_packet_number)
	{
		cout<<"\n\nRECIEVED ALL PACKETS...\n";				
		return true;
	}	
	else
		return false;
}

void update_map()
{
	for(it = recieved_map.find(last_packet_received);it!=recieved_map.end();)
	{	
		it++;
		if(it->first==last_packet_received+1)
		{
				last_packet_received++;
		}
		else
		{
			return;
		}
	}
}


void *check_map(void *arg1)
{
	sockfd_s = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd_s < 0)
		error("ERROR opening socket");
	server_s = gethostbyname(client_name);
	if (server_s == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char*) &serv_addr_s, sizeof(serv_addr_s));
	serv_addr_s.sin_family = AF_INET;
	bcopy((char*)server_s->h_addr,
		(char*)&serv_addr_s.sin_addr.s_addr,
		 server_s->h_length);
	serv_addr_s.sin_port = htons(PORTNUMBER);
	servlen_s = sizeof(serv_addr_s);

	while(1)
	{
		usleep(ack_sleep_time);		
		char *buffer2 = (char *)calloc(sizeof(char)*5, 1);
		cout<<"\n************************Sending ACK of "<<last_packet_received+1<<endl;

		int lpr;
		if(check_all_packets_recieved())
			lpr = htonl(-1);	
		else
			lpr= htonl(last_packet_received+1);

		memcpy(buffer2, &lpr,sizeof(int));
		int n = sendto(sockfd_s, buffer2, sizeof(int)*1, 0, (struct sockaddr*) &serv_addr_s, servlen_s);
		if (n < 0)
		{	error("ERROR on sendto!"); 
			exit(0);
		}
		free(buffer2);
	    
	}
}


int main(int argc, char *argv[1])
{
	
	pthread_t check_map_thread;
	bool first_run=true;

	bool true_flag=true,false_flag=false;
	
	int n;
	signal(SIGCHLD,SIG_IGN);
	if (argc < 2) {

		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	
	client_name=argv[1];
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {	
		error("ERROR opening socket");
	}

	bzero((char*)&serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[2]));
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	
	clilen = sizeof(cli_addr);
	
	while(1) 
	{ 

		char *data;
		char buffer[PACKETSIZE];
		bzero(buffer,PACKETSIZE);
		n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
		if (n < 0)
			error("ERROR on recvfrom");		
		
		sequence_number =  get_sequence_number(buffer);
		data=get_data(buffer);
		
		cout<<"\nREAL_THREAD_Received Packet no "<<sequence_number<<endl;

		it = recieved_map.find(sequence_number);
		if ( it==recieved_map.end()) 
		{
			cout<<"\ninserting packet "<<sequence_number<<endl;
			recieved_map.insert(pair<int,char*>(sequence_number,data));			
			if(sequence_number == (last_packet_received + 1) )
			{
				last_packet_received=sequence_number;
				cout<<"\nIncrementing the lpr to "<<last_packet_received<<endl;
				update_map();
			}
			else
			{
				if(retrans_flag)
				{
					retrans_flag=false;
					int rett=pthread_create(&check_map_thread,NULL,check_map,NULL);
					//int iret1 = pthread_create( &listener_thread, NULL,create_listener,NULL);			
				}
			}			
			
			if(strlen(data)<DATASIZE)
			{
				last_packet_number=get_sequence_number(buffer);
				cout<<"\n\nTHE LAST PACKET IS "<<last_packet_number<<endl;
			}
			if(check_all_packets_recieved())
				print_data();
		}
		else
		{
			cout<<"\n dropping packet "<<sequence_number<<endl;
		}
		free(data);
		
	}
	close(sockfd);
	return 0;
}


void print_data()
{

	cout<<"\n\n\nWRITING DATA....\n\n";	
	FILE *fp = fopen("output.txt", "wb");
	for(it=recieved_map.begin();it!=recieved_map.end();it++)
	{	
		cout<<it->second;
		fwrite(it->second,1,strlen(it->second),fp);
		
	}	
	fclose(fp);
	cout<<"\n"<<recieved_map.size()<<endl;
	exit(0);
}