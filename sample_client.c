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
   char sendline[1000];
   char recvline[1000];

   if (argc != 2)
   {
      printf("usage:  udpcli <IP address>\n");
      exit(1);
   }

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(PORTNUMBER);

   while (fgets(sendline, 100,stdin) != NULL)
   {
      sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
      n=recvfrom(sockfd,recvline,100,0,NULL,NULL);
      recvline[n]=0;
      fputs(recvline,stdout);
   }
}



int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[100];
   char recvline[100];

  
   sockfd=socket(AF_INET,SOCK_DGRAM,0);
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(PORTNUMBER);
   
   if(fork()!=0)
   {
      while (fgets(sendline, 100,stdin) != NULL)
      {
         sendto(sockfd,sendline,strlen(sendline),0,
                (struct sockaddr *)&servaddr,sizeof(servaddr));
      }
   }
   else
   {
      while(1)
      {
         n=recvfrom(sockfd,recvline,100,0,NULL,NULL);
         recvline[n]=0;
         fputs(recvline,stdout);
      }
   }





}