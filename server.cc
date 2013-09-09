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

int ack_sleep_time=100000;


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
map<int, char*>::reverse_iterator rit;
map<int, char*> storing_map;


int last_packet_received = 0; //last inorder received
int last_out_of_order_packet_received = 0; //last inorder received
int last_packet_number=-1;
int sequence_number;


void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void print_data();

bool check_all_packets_recieved()
{
	//cout<<"\nSECOND_THREAD_Map size is "<<recieved_map.size()<<endl;
	if(recieved_map.size()==last_packet_number)
	{
		cout<<"\n\nRECIEVED ALL PACKETS...\n";				
		print_data();
		char *buffer2 = (char *)calloc(sizeof(char)*5, 1);	   
		int temp=htonl(0);
		memcpy(buffer2, &temp, sizeof(int));
		int n = sendto(sockfd_s, buffer2, sizeof(int)*1, 0, (struct sockaddr*) &serv_addr_s, servlen_s);
		if (n < 0)
		{	error("ERROR on sendto!"); 
			exit(0);
		}
	    if (n < 0) 
	        error("ERROR writing to socket");    
	       
	    free(buffer2);
	    return true;
	}	
	else
	{
		//cout<<"\n\nNOT RECIEVED ALL PACKETS...\n";
		return false;
	}

}


void *check_map(void* arg1)
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
		if(check_all_packets_recieved())
			print_data();
		
			char *buffer2 = (char *)calloc(sizeof(char)*5, 1);
			//cout<<"\nSending data..."<<last_packet_received<<"\t"<<it->first<<endl;
			int lpr = htonl(last_packet_received+1);	
			memcpy(buffer2, &lpr,sizeof(int));
			int n = sendto(sockfd_s, buffer2, sizeof(int)*1, 0, (struct sockaddr*) &serv_addr_s, servlen_s);
			if (n < 0)
			{	error("ERROR on sendto!"); 
				exit(0);
			}
		    free(buffer2);
	    usleep(ack_sleep_time);
	}

}


int main(int argc, char *argv[1])
{
	
	pthread_t check_map_thread,update_map;
	bool first_run=true;

	bool true_flag=true,false_flag=false;
	
	int n;
	signal(SIGCHLD,SIG_IGN);
	if (argc < 2) {

		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	portno = atoi(argv[2]);
	client_name=argv[1];
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {	
		
		error("ERROR opening socket");

	}

	bzero((char*)&serv_addr, sizeof(serv_addr));
	
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
	
	while(1) 
	{ 

		char *data;
		char buffer[PACKETSIZE];
		bzero(buffer,PACKETSIZE);
		n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
		if (n < 0)
			error("ERROR on recvfrom");		
		
		//Extracting the data from the packet
		sequence_number =  get_sequence_number(buffer);
		data=get_data(buffer);
		
		//cout<<"\n\nMap size is "<<recieved_map.size()<<endl;
		cout<<"\nREAL_THREAD_Received Packet no "<<sequence_number<<endl;

		it = recieved_map.find(sequence_number);
		if ( it==recieved_map.end()) {
			//cout<<"\ninserting packet "<<sequence_number<<endl;
			recieved_map.insert(pair<int,char*>(sequence_number,data));			
			if(sequence_number == (last_packet_received + 1) )
			{
				last_packet_received = sequence_number;
			}
			else
			{
				if(sequence_number>last_out_of_order_packet_received)
					last_out_of_order_packet_received=sequence_number;

				if(retrans_flag)
				{
						int iret1 = pthread_create( &check_map_thread, NULL,check_map, NULL);
						retrans_flag=false;
				}
			}			
			
			if(strlen(data)<DATASIZE || last_packet_number!=-1)
			{
				if(last_packet_number==-1)
					last_packet_number=get_sequence_number(buffer)+1;
				
				cout<<"\n\nTHE LAST PACKET IS "<<last_packet_number<<endl;
				if(check_all_packets_recieved())
				{
					print_data();
				}

			}	

		}
		else
		{
			cout<<"\n dropping packet "<<sequence_number<<endl;
		}
		free(data);
		
	}
	close(sockfd);
	fclose(fd);
	return 0;
}


void print_data()
{

	cout<<"\n\n\nWRITING DATA....\n\n";	
	FILE *fp = fopen("output.txt", "w+");
	for(it=recieved_map.begin();it!=recieved_map.end();it++)
	{	
		fwrite(it->second,1,strlen(it->second),fp);
		cout<<it->second;
	}	
	fclose(fp);
	cout<<"\n"<<recieved_map.size()<<endl;
	exit(0);
}