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
using namespace std;

#define PACKETSIZE 1400
#define DATASIZE (PACKETSIZE - sizeof(int))
#define TCP_PORT 11000
#define INITIAL_PORT 13000
#define SOCK2 7001
#define SOCK3 7002
#define DELAY 30
#define ITERATOR 1
int flag =1;
int invert = 0;
static int sequence_number = 1;
int total_packets = 0; 
void append_sequence_number(char *packet, int sequence_number)
{
   
    int x = htonl(sequence_number);
    memcpy(packet + 0, &x, sizeof(int));
}
void error(const char* msg)
{
    perror(msg);
    exit(0);

}


 
void send_total_packets(int total, int remainder, char receiver[NI_MAXHOST])
{

  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[10];
  
  portno = INITIAL_PORT;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");
  server = gethostbyname(receiver);
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
  if( connect(sockfd, (struct sockaddr*) &serv_addr,
	      sizeof(serv_addr)) < 0)
    error("ERROR connecting");
  bzero(buffer, 10);
  
  int x = htonl(total);
  memcpy(buffer,&x, sizeof(int));

  int y = htonl(remainder);
  memcpy(buffer+sizeof(int),&y,sizeof(int));

  n = write(sockfd, buffer, 2*sizeof(int));
  if (n < 0)
    error("ERROR writing to socket");


	close(sockfd);

}


void * tcp_listener(void *arg)
{
	int sockfd, newsockfd, portno;
	pid_t pid;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {	
		
		error("ERROR opening socket");

	}
	bzero((char*)&serv_addr, sizeof(serv_addr));
	portno = TCP_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd,10);
	clilen = sizeof(cli_addr); 
	
	newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,
			&clilen);
	if (newsockfd < 0)
			error("ERROR on accept");

	  bzero(buffer, 256);	
	  n = read(newsockfd, buffer, 255);
	  if (n < 0)
        	error("ERROR reading from socket");
                
	  //printf("\nMessage from %s\n", buffer);
	  if((strncmp(buffer, "END",3) == 0)){

	    n = write(newsockfd,"END Rec\n", 4);
	    flag = 0;
	    if (n < 0 )
		error("ERROR sending on socket");
	
	}

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
    //cout<<"REM"<<remainder<<endl; 
    
    if((sizeoffile%DATASIZE) ==0)
	{

		remainder = DATASIZE;
	}

    send_total_packets(total_packets, remainder,argv[2]); 
    pthread_t tcp_listen;
    pthread_create(&tcp_listen, NULL, tcp_listener, NULL);
    
    int last_packet_no = 0;
    
    
    
    int sockfd, portno,sockfd_1, portno_1,sockfd_2, portno_2, n,n_1,n_2,data_read,iterator=2;
    struct sockaddr_in serv_addr,serv_addr_1,serv_addr_2;
    struct hostent *server,*server_1,*server_2;
    socklen_t servlen,servlen_1,servlen_2;
    char *buffer;
    char filename[256];
    bzero(filename,256);
 
    if (argc < 4) {
     
        fprintf(stderr, "usage %s <source> <destination>  <port1> \n", argv[0]);
        exit(0);
 
    }
 
 
 
    portno = atoi(argv[3]);
    portno_1 = SOCK2;
    portno_2 = SOCK3;
     
 /********************************************/
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
    
/************************************************/
   sockfd_1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_1 < 0)
        error("ERROR opening socket");
    server_1 = gethostbyname(argv[2]);
    if (server_1 == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
         
    }
    bzero((char*) &serv_addr_1, sizeof(serv_addr_1));
    serv_addr_1.sin_family = AF_INET;
    bcopy((char*)server_1->h_addr,
          (char*)&serv_addr_1.sin_addr.s_addr,
          server_1->h_length);
    serv_addr_1.sin_port = htons(portno_1);
    servlen_1 = sizeof(serv_addr_1);
    
/************************************************/
/************************************************/
   sockfd_2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_2 < 0)
        error("ERROR opening socket");
    server_2 = gethostbyname(argv[2]);
    if (server_2 == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
         
    }
    bzero((char*) &serv_addr_2, sizeof(serv_addr_2));
    serv_addr_2.sin_family = AF_INET;
    bcopy((char*)server_2->h_addr,
          (char*)&serv_addr_2.sin_addr.s_addr,
          server_2->h_length);
    serv_addr_2.sin_port = htons(portno_2);
    servlen_2 = sizeof(serv_addr_2);
    
/************************************************/


    
    
    
    
    
      
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
			//printf("Loop again\n");
			fseek(fd,0,SEEK_SET);
			sequence_number = 1;

//        	printf("Inverted\n");
//        	invert =1;
//        	last_packet_no = sequence_number;
//        	append_sequence_number(buffer, last_packet_no);
//     	n = sendto(sockfd, buffer, 4, 0, (struct sockaddr*) &serv_addr, servlen);
//     	n_1 = sendto(sockfd_1, buffer, 4, 0, (struct sockaddr*) &serv_addr_1, servlen_1);
//     	n_2 = sendto(sockfd_1, buffer, 4, 0, (struct sockaddr*) &serv_addr_1, servlen_1);
//        	sequence_number--;
//        	continue;	
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
     		/*n = sendto(sockfd, buffer, 4, 0, (struct sockaddr*) &serv_addr, servlen);
     		n_1 = sendto(sockfd_1, buffer, 4, 0, (struct sockaddr*) &serv_addr_1, servlen_1);
     		n_2 = sendto(sockfd_1, buffer, 4, 0, (struct sockaddr*) &serv_addr_1, servlen_1);*/
        	  	continue;
        	  }
        	  
        }
        printf("sending seq_num %d\n",(sequence_number-1));
       while(iterator>0)
       {
		
       	n = sendto(sockfd, buffer, m+sizeof(int), 0, (struct sockaddr*) &serv_addr, servlen);
        	
        	n_1 = sendto(sockfd_1, buffer, (m+sizeof(int)), 0, (struct sockaddr*) &serv_addr_1, servlen_1);
        	
        	n_2 = sendto(sockfd_2, buffer, (m+sizeof(int)), 0, (struct sockaddr*) &serv_addr_2, servlen_2);
        	usleep(DELAY);
        	iterator--;
     }
      
        counter = n+counter;
        if (n < 0)
            error("ERROR on sendto");
            free(buffer);
             
             
    }
    flag=1;

    if(pthread_join(tcp_listen, NULL)) {

	fprintf(stderr, "Error joining thread\n");
	return 2;

	}
    close(sockfd);
   fclose(fd);
      
    return 0;
 
}
