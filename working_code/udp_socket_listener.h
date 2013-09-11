class udp_socket_listener{
	int sockfd;
	struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

public:
	udp_socket_listener(int port_number);   	
    int recieve_packet_through_socket(char *buffer,int packet_size);
    void close_socket();
};


int udp_socket_listener::recieve_packet_through_socket(char *buffer,int packet_size)
{
	int n = recvfrom(sockfd, buffer, PACKETSIZE, 0,(struct sockaddr*) &cli_addr, &clilen);
    if (n < 0)
       perror("ERROR on recvfrom");
  	return n;
}


void udp_socket_listener::close_socket()
{
	close(sockfd);
}

udp_socket_listener::udp_socket_listener(int port_number)
{
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {   
        perror("ERROR opening socket");
 	}    
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        perror("ERROR on binding");
    clilen = sizeof(cli_addr);
}



