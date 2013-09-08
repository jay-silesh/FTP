#include "Packet.h"
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

int get_sequence_number(char *packet)
{
	int seq, x;
	memcpy(&seq, packet, sizeof(int));
	x = ntohl(seq);
	return x;	
}


char* get_data(char *packet)
{
	char *buffer = (char *)calloc(sizeof(char)*(DATASIZE), 1);
	memcpy(buffer, packet+sizeof(int), DATASIZE);
	return buffer;	
}
