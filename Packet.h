typedef struct
{
	bool last_packet;
	int seq_no;
	char data[256];
}Packet;
