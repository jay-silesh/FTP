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


char addr_str[INET_ADDRSTRLEN+1];

char * client_name;

int sockfd, portno;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;


int sockfd2;
struct sockaddr_in client_addr2;
struct hostent *client2;
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





void create_retransmission_connection()
{

    client2 = gethostbyname(client_name);
    if (client2 == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &client_addr2, sizeof(client_addr2));
    client_addr2.sin_family = AF_INET;

    bcopy((char *)client2->h_addr, 
         (char *)&client_addr2.sin_addr.s_addr,
         client2->h_length);
    
    client_addr2.sin_port = htons(PORTNUMBER);
    
    sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd2 < 0) 
        error("ERROR opening socket");
    
    if (connect(sockfd2,(struct sockaddr *) &client_addr2,sizeof(client_addr2)) < 0) 
        error("ERROR connecting");
	    
}






void print_data()
{

	cout<<"\n\n\nWRITING DATA....\n\n";	
	FILE *fp = fopen("output.txt", "w");
	for(it=recieved_map.begin();it!=recieved_map.end();it++)
	{
		//cout<<"\n\n**************\n\n"<<"Pakcet no:"<<it->first<<"-->"<<it->second;
		fwrite(it->second,1,DATASIZE,fp);
		
	}
	fwrite("\0",1,1,fp);
	fclose(fp);
	cout<<"\n"<<recieved_map.size()<<endl;
	exit(0);
}


bool check_all_packets_recieved()
{
	cout<<"\nMap size is "<<recieved_map.size()<<endl;
	if(recieved_map.size()==last_packet_number)
	{
		cout<<"\n\nRECIEVED ALL PACKETS...\n";				
		char *buffer2 = (char *)calloc(sizeof(char)*10, 1);	   
		int temp=htonl(0);
		memcpy(buffer2, &temp, sizeof(int));
		memcpy(buffer2 + 4, &temp, sizeof(int));
		int n = write(sockfd2,buffer2,sizeof(int)*2);
	    if (n < 0) 
	        error("ERROR writing to socket");    
	       
	    close(sockfd2);

	    return true;
	}	
	else
	{
		cout<<"\n\nNOT RECIEVED ALL PACKETS...\n";
		return false;
	}

}


void *check_map(void* flag_retran)
{

	bool *flag_retrans=(bool*)flag_retran;
		

	if(check_all_packets_recieved())
		print_data();

	if(*flag_retrans)
		cout<<"\n\nRetransmission packets..........\n\n";
	else
		cout<<"\n\nUpdating map.........\n\n";

	cout<<"\n\nFlag got is "<<*flag_retrans<<endl;
	//sleep(5);

	for(it = recieved_map.find(last_packet_received);it!=recieved_map.end();)
	{	
		it++;
		if(it->first==last_packet_received+1)
		{
			last_packet_received++;
		}
		else
		{
			if(*flag_retrans==true)
			{

				cout<<"\n\nRECIEVED ALL PACKETS...\n";				
				char *buffer2 = (char *)calloc(sizeof(char)*10, 1);
				cout<<"\nSending data..."<<last_packet_received<<"\t"<<it->first<<endl;
				int lpr = htonl(last_packet_received);	
				int temp=htonl(it->first);
				memcpy(buffer2, &lpr,sizeof(int));
				memcpy(buffer2 + 4, &temp,sizeof(int) );		
				
				int n = write(sockfd2,buffer2,sizeof(int)*2);
			    if (n < 0) 
			        error("ERROR writing to socket");    
				cout<<"\n\n\n****************Finsihed retrans...******************\n";
	    		//sleep(1);	       
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


	client_name=argv[2];


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
		
		
		
		//Extracting the data from the packet
		sequence_number =  get_sequence_number(buffer);
		data=get_data(buffer);
		cout<<"\nReceived Packet no "<<sequence_number<<"\tReceived data bytes"<<strlen(data)<<endl;
		
		cout<<"\n\nMap size is "<<recieved_map.size()<<endl;


		it = recieved_map.find(sequence_number);
		if ( it==recieved_map.end()) {
			cout<<"\ninserting packet "<<sequence_number<<endl;
			recieved_map.insert(pair<int,char*>(sequence_number,data));			
			if(sequence_number == (last_packet_received + 1) )
			{

				last_packet_received = sequence_number;
				if(check_flag)
				{
					check_flag=false;
					cout<<"\nNOT Creating a thread and .....Sending true flag";
					//sleep(5);
					if(retrans_flag)
					{
						create_retransmission_connection();
						retrans_flag=false;
					}
					check_map((void*)&false_flag);

					//iret2 = pthread_create( &update_map_thread, NULL, update_map, NULL);	
				}
				
			}
			else
			{
				check_flag=true;
				//out of order module...
				if(sequence_number>last_out_of_order_packet_received)
					last_out_of_order_packet_received=sequence_number;

				if(last_out_of_order_packet_received==last_packet_received)
					check_flag=false;
				cout<<"\nCreating a thread and .....Sending true flag";
				if(retrans_flag)
				{
						create_retransmission_connection();
						retrans_flag=false;
				}
				int iret1 = pthread_create( &check_map_thread, NULL,check_map, (void*)&true_flag);
				//sleep(5);
				
			}			
			counter = counter + n;	
		

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
			cout<<"\n dropped packet "<<sequence_number<<endl;
		}
		
	}

	


	close(sockfd);
	fclose(fd);
	return 0;
}








