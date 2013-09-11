class udp_socket_sender{
	int sockfd;
	struct sockaddr_in serv_addr;
    struct hostent *server;
    socklen_t servlen;

public:
	udp_socket_sender(int port_number,char *host_name);   	
    int send_packet_through_socket(char *buffer,int packet_size);
    void close_socket();
};


int udp_socket_sender::send_packet_through_socket(char *buffer,int packet_size)
{
	int n = sendto(sockfd, buffer,packet_size,0,(struct sockaddr*) &serv_addr, servlen);
	return n;
}
void udp_socket_sender::close_socket()
{
	close(sockfd);
}

udp_socket_sender::udp_socket_sender(int port_number,char *host_name)
{
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("Error in socket creation...");
		exit(1);
	}
    server = gethostbyname(host_name);
	if (server == NULL)
    	fprintf(stderr, "ERROR, no such host\n");
     
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,
      (char*)&serv_addr.sin_addr.s_addr,
      server->h_length);
	serv_addr.sin_port = htons(port_number);
	servlen = sizeof(serv_addr);
}



