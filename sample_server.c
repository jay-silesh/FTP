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


using namespace std;

#define PACKETSIZE 100

#define PORTNUMBER 6000


void error(const char* msg)
{
   perror(msg);
   exit(0);

}

int main3()
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      printf("Received the following:\n");
      printf("%s",mesg);
      printf("-------------------------------------------------------\n");
   }
}

int main(int argc, char**argv)
{
   char * client_name;
   char buffer[100];

   int sockfd, portno;
   socklen_t clilen;
   struct sockaddr_in serv_addr, cli_addr;

   client_name=argv[3];
   sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if (sockfd < 0) { 
      
      error("ERROR opening socket");

   }
   bzero((char*)&serv_addr, sizeof(serv_addr));
   portno = PORTNUMBER;
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
      error("ERROR on binding");
   
   clilen = sizeof(cli_addr);
   bzero(buffer,PACKETSIZE);
   int n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
   cout<<buffer<<endl;   

   if(fork()==0)
   {
         while(1)
         {
            
            int sockfd2;
            struct sockaddr_in servaddr2;
            socklen_t len2;   
            sockfd2=socket(AF_INET,SOCK_DGRAM,0);
            bzero(&servaddr2,sizeof(servaddr2));
            servaddr2.sin_family = AF_INET;
            servaddr2.sin_addr.s_addr=htonl(gethostbyname(client_name));
            servaddr2.sin_port=htons(PORTNUMBER+5);

            char *buffer1 = (char *)calloc(sizeof(char)*10, 1);
            int lpr = htonl(5); 
            int temp=htonl(6);
            memcpy(buffer1, &lpr, sizeof(int));
            memcpy(buffer1 + 4, &temp, sizeof(int));      
            sleep(5);
            sendto(sockfd2, buffer1, sizeof(int)*2, 0, (struct sockaddr*) &servaddr2, sizeof(servaddr2));
            close(sockfd2);
            
         }
      }
      else
      {
         while(1)
         {

            bzero(buffer,PACKETSIZE);
            n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
            cout<<buffer<<endl;

         }
      }


      return 1;

}



















