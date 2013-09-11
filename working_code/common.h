#define PACKETSIZE 1400
#define DATASIZE (PACKETSIZE - sizeof(int))
#define TCP_PORT 11000
#define INITIAL_PORT 13000
#define SOCK2 7001
#define SOCK3 7002
#define SOCK4 7003
#define SOCK5 7004
#define DELAY 60
#define ITERATOR 1


void append_sequence_number(char *packet, int sequence_number)
{ 
    int x = htonl(sequence_number);
    memcpy(packet + 0, &x, sizeof(int));
}

int get_sequence_number(char *packet)
{
    
    int seq, x;    
    memcpy(&seq, packet, sizeof(int));
    x = ntohl(seq);
    return x;   
}
