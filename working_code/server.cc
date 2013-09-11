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
#include <queue>
#include <signal.h>
#include <netdb.h> 
#include <time.h>
#include <sys/time.h>
#include "common.h"
#include "udp_socket_listener.h"
#include "udp_socket_sender.h"
#include "tcp_socket_listener.h"
#include "tcp_socket_sender.h"


using namespace std;
queue<char *> data_queue;
pthread_mutex_t file_mutex;
pthread_t filewrite, secondsock, thirdsock,fourthsock,fifthsock;
FILE *fd;
struct timeval start;
struct timeval end;
time_t transfer_time;
map<int,char*>sequence_map;
map<int,char*>::iterator it;
char client[NI_MAXHOST];
int total_packets = 0;
int last_length = 0;
struct sockaddr_in  cli_addr_global;



void sig_handler(int signo)
{
 if (signo == SIGINT)
 fclose(fd);
   printf("received SIGINT\n");
   exit(0);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);

}

int get_total_packets()
{
	int pkts;
    char buffer[10];
	bzero(buffer, 10);

    tcp_socket_listener tcp_socket1(INITIAL_PORT);
	tcp_socket1.read_from_tcp_socket(buffer,2*sizeof(int));

	memcpy(&pkts, buffer, sizeof(int));
    int x = ntohl(pkts);
	int y = 0;
	memcpy(&y, buffer+sizeof(int), sizeof(int));
	last_length = ntohl(y);
	
    cli_addr_global=tcp_socket1.get_sockaddr();
    tcp_socket1.close_tcp_listener_socket();
    tcp_socket1.newclose_tcp_listener_socket();
    return x;
}


 
void Terminate(char * server_ip)
{
	printf("Terminate started\n");
    char buffer[256],buffer_1[256];    
    tcp_socket_sender tcp_socket2(TCP_PORT,&cli_addr_global);
    printf("Connected to Sender\n ");
    char BUffer[4];
	strcpy(BUffer,"END");
    	    
    tcp_socket2.write_to_tcp_socket(BUffer,strlen(BUffer));
    bzero(BUffer,sizeof(BUffer));
	    
	tcp_socket2.read_from_tcp_socket(BUffer,255);
    tcp_socket2.close_tcp_sender_socket(); 
  
    pthread_cancel(secondsock);
    pthread_cancel(thirdsock);
    pthread_cancel(fourthsock);
    pthread_cancel(fifthsock);
    exit(0);
   
}

void *func_sock(void * portno)
{
	if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
 
    char *buffer;
    udp_socket_listener udp_socket2(*(int*)portno);
    
    int counter=0;
    while(1)
    {
        buffer =(char *)calloc(sizeof(char)*(PACKETSIZE), 1);
        int n=udp_socket2.recieve_packet_through_socket(buffer,PACKETSIZE);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        int x,sequence_number;
        memcpy(&x, buffer, sizeof(int));
    	sequence_number=ntohl(x);
    	
        pthread_mutex_lock(&file_mutex);
    	if(sequence_map.find(sequence_number)==sequence_map.end())
    	{
    	   sequence_map.insert(pair<int,char*>(sequence_number,buffer));
    			
    	}
    	else
    	{
    		free(buffer);
    	}
		pthread_mutex_unlock(&file_mutex);
 
    }
 
}
    
    
    

void *file_write(void *arg)
{
    int seq_num=1;
    char server_IP[255]="localhost";
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
 
	fd = fopen("output","w");
    if(fd==NULL)
    {
        printf("Error while opening file\n");
        exit(1);
    }
    char * buffer;

 while(1)
 {
	   pthread_mutex_lock(&file_mutex);
	   it = sequence_map.find(seq_num);
	   if(it!=sequence_map.end())
	   {
	 	    buffer = it->second;
	       	pthread_mutex_unlock(&file_mutex);
            seq_num++;

            if(seq_num == (total_packets + 1) )
		    {
			
    			fwrite(buffer+sizeof(int),sizeof(char),last_length,fd);
    			gettimeofday(&end,NULL);
    			transfer_time  = (end.tv_sec - start.tv_sec);
			
    			float throughput =((( total_packets*DATASIZE)*8)/transfer_time);
    			FILE *result = fopen("result","w");
    			stringstream ss;
    			char temp[2000];
    			bzero(temp,2000);
    			ss<<"Transfer time: "<<transfer_time<<"seconds\n Throughput"<<throughput<<"bps\n";
    			ss>>temp;
    			fwrite(temp,1,2000,result);
    			fclose(result);
    			cout<<"Transfer time: "<<transfer_time<<"seconds\n Throughput"<<throughput<<"bps\n";
    			fclose(fd);
    	 		Terminate(client);
	 		
	 		    break;
	 		
		    }
    		printf("sequence num written %d\n",seq_num);
    		fwrite(buffer+sizeof(int),sizeof(char),PACKETSIZE-sizeof(int),fd);
    	 	free(buffer);
	
	 }
	 else
	 {
	 	 
	 	 pthread_mutex_unlock(&file_mutex);
	 }
   }
 
}
 




 
int main(int argc, char *argv[])
{
    
   int port1=7001;
   int port2=7002;
   int port3=SOCK4;
   int port4=SOCK5;

	
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");





    pthread_mutex_init(&file_mutex,NULL);
	if(pthread_create(&secondsock, NULL, func_sock, (void *)&port1)) 
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    
    if(pthread_create(&thirdsock, NULL, func_sock, (void *)&port2))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if(pthread_create(&fourthsock, NULL, func_sock, (void *)&port3))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    if(pthread_create(&fifthsock, NULL, func_sock, (void *)&port4)) 
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
	total_packets = get_total_packets();
	
	gettimeofday(&start, NULL);

   
   if(pthread_create(&filewrite, NULL, file_write, NULL)) {

	fprintf(stderr, "Error creating thread\n");
	return 1;

	} 
	
    pid_t pid;
    int last_packet_received = 0; //last inorder received
    
    int sequence_number;
    
    signal(SIGCHLD,SIG_IGN);
    if (argc < 2) {
 
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
     
    udp_socket_listener udp_socket3(atoi(argv[1]));    
        
    int counter=0;
    while(1)
    {
        char *buffer =(char *)calloc(sizeof(char)*(PACKETSIZE), 1);
        int n=udp_socket3.recieve_packet_through_socket(buffer,PACKETSIZE);
        int x;
		memcpy(&x, buffer, sizeof(int));
    	sequence_number=ntohl(x);
    	
    	pthread_mutex_lock(&file_mutex);
		if(sequence_map.find(sequence_number)==sequence_map.end())
		{
			
			sequence_map.insert(pair<int,char*>(sequence_number,buffer));
			
		}
		else
		{
			free(buffer);
		}
		pthread_mutex_unlock(&file_mutex);
        counter = counter + n; 
    }
 
    
    printf("%d bytes received\n",counter);
    udp_socket3.close_socket();

    if(pthread_join(filewrite, NULL))
    {
    	fprintf(stderr, "Error joining thread\n");
    	return 2;
	}
	if(pthread_join(secondsock, NULL)) 
    {
    	fprintf(stderr, "Error joining thread\n");
    	return 2;
	}
	if(pthread_join(thirdsock, NULL)) 
    {
    	fprintf(stderr, "Error joining thread\n");
    	return 2;
	}
    if(pthread_join(fourthsock, NULL)) 
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
    if(pthread_join(fifthsock, NULL)) 
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
	

    return 0;
}
