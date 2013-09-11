#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <queue>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <pthread.h>
#include "udp_socket_sender.h"
#include "tcp_socket_listener.h"
#include "tcp_socket_sender.h"
#include "common.h"

using namespace std;


int flag =1;
int invert = 0;
static int sequence_number = 1;
int total_packets = 0;
char *file_name;

pthread_t inverted_thread; 

void error(const char* msg)
{
    perror(msg);
    exit(0);

}

void send_total_packets(int total, int remainder, char receiver[NI_MAXHOST])
{

  char buffer[10];
  bzero(buffer, 10);

  int portno = INITIAL_PORT;
  tcp_socket_sender tcp_socket2(portno,receiver);
  
  int x = htonl(total);
  memcpy(buffer,&x, sizeof(int));

  int y = htonl(remainder);
  memcpy(buffer+sizeof(int),&y,sizeof(int));

  int n= tcp_socket2.write_to_tcp_socket(buffer,sizeof(int)*2);
	tcp_socket2.close_tcp_sender_socket();

}


void * tcp_listener(void *arg)
{
	char buffer[256];
	bzero(buffer,256);
  tcp_socket_listener tcp_socket1(TCP_PORT);
  tcp_socket1.read_from_tcp_socket(buffer,255);
		
  if((strncmp(buffer, "END",3) == 0))
  {
    int n = tcp_socket1.write_to_tcp_socket("END Rec\n", 4);
    flag = 0;
    if (n < 0 )
	    error("ERROR sending on socket");
	}

}


void *inversion_send(void *arg1)
{
  char *arg2=(char*)arg1;
  udp_socket_sender socket3(SOCK3,arg2);
  udp_socket_sender socket4(SOCK4,arg2);
  int inverted_sequence_number=total_packets;
  int n,n_1;
  FILE *fd = fopen(file_name,"r+");
  if(fd == NULL)
  {
      error("ERROR IN OPENING A FILE in the inverted thread...");

  }

  char *buffer;
  while(1)
  {
     
       iterator=ITERATOR;
       buffer = (char *)malloc(sizeof(char)*(PACKETSIZE));
       append_sequence_number(buffer,inverted_sequence_number);
       
       fseek(fd,(inverted_sequence_number-1)*DATASIZE,SEEK_SET); 
       m = fread(buffer + sizeof(int), 1, DATASIZE,fd);
       inverted_sequence_number--;
       if(inverted_sequence_number== 0)
       {
          inverted_sequence_number = total_packets;
          continue;
       }
            
       printf("sending seq_num %d\n",(sequence_number-1));
        
       while(iterator>0)
       {
        
            n=socket3.send_packet_through_socket(buffer,m+sizeof(int));
            n_1=socket4.send_packet_through_socket(buffer,m+sizeof(int));
            usleep(DELAY);
            iterator--;
       }   
        
       free(buffer);
             
             
    }
    
    socket3.close_socket();
    socket4.close_socket();
    fclose(fd);
}



 
int main(int argc, char* argv[])
{
   
    int remainder;

    FILE *size = fopen(argv[1],"r");
    fseek(size,0,SEEK_END);
    int sizeoffile = ftell(size);
    fclose(size);
    total_packets = sizeoffile/DATASIZE;
    if((sizeoffile%DATASIZE) != 0)
	   total_packets++;
   
    remainder = (sizeoffile - ((total_packets-1)*DATASIZE));
    
    if((sizeoffile%DATASIZE) ==0)
	  {
      remainder = DATASIZE;
	  }

    send_total_packets(total_packets, remainder,argv[2]); 
    pthread_t tcp_listen;
    pthread_create(&tcp_listen, NULL, tcp_listener, NULL);


    int last_packet_no = 0;
       
    int portno,portno_1,portno_2, n,n_1,n_2,data_read,iterator=2;
    char *buffer;
    char filename[256];
    bzero(filename,256);
 
    if (argc < 4)
    {
        fprintf(stderr, "usage %s <source> <destination>  <port1> \n", argv[0]);
        exit(0);
    }

    file_name=argv[1];
    //This thread should be called after initializing the global variable total_packets
    pthread_create(&inverted_thread, NULL, inversion_send, (void*)argv[2]);
 
    portno = atoi(argv[3]);    
    
    udp_socket_sender socket1(portno,argv[2]);
    udp_socket_sender socket2(SOCK2,argv[2]);
        

    FILE *fd = fopen(argv[1],"r+");
    if(fd == NULL)
    {
        error("ERROR IN OPENING A FILE");
 
    }
    int m=1,counter=0;
    while(flag)
    {
     
       iterator=ITERATOR;
       buffer = (char *)malloc(sizeof(char)*(PACKETSIZE));
	     append_sequence_number(buffer, sequence_number);
       if(!invert)
       {
          	m = fread(buffer + sizeof(int), 1, DATASIZE,fd);
  	        if(m<DATASIZE )
  		      {
  			       buffer[m+sizeof(int)]='\0';
  		      }
  		      
            sequence_number++;
            if(m==0)
            {
          			fseek(fd,0,SEEK_SET);
          			sequence_number = 1;
                continue;
            }
        }
        else
        {
        	  fseek(fd,(sequence_number-1)*DATASIZE,SEEK_SET); 
        	  m = fread(buffer + sizeof(int), 1, DATASIZE,fd);
        		sequence_number--;
        	  if(sequence_number == 0)
        	  {
        	  	printf("normal way\n");
        	  	invert = 0;
        	  	fseek(fd,0,SEEK_SET);
        	  	sequence_number++;
        	  	append_sequence_number(buffer, last_packet_no);
           		continue;
        	  }
        	  
        }
        printf("sending seq_num %d\n",(sequence_number-1));
        
        while(iterator>0)
        {
		    
          n=socket1.send_packet_through_socket(buffer,m+sizeof(int));
          n_1=socket2.send_packet_through_socket(buffer,m+sizeof(int));
          usleep(DELAY);
        	iterator--;
        }
      
        counter = n+counter;
        if (n < 0)
          error("ERROR on sendto");
        
        free(buffer);
             
             
    }
    flag=1;

    if(pthread_join(tcp_listen, NULL)) 
    {
	    fprintf(stderr, "Error joining thread\n");
      return 2;
    }

   socket1.close_socket();
   socket2.close_socket();
   fclose(fd);
      
   return 0;
 
}
