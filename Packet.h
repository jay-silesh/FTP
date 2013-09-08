
#define PACKETSIZE 1400
#define DATASIZE PACKETSIZE-sizeof(int)
#define HAEDERSIZE sizeof(int)

typedef struct
{
	bool last_packet;
	int seq_no;
	char data[256];
}Packet;

int get_sequence_number(char *packet);
char* get_data(char *packet);
