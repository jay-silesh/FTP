class tcp_socket_sender{
	int sockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	
public:
	tcp_socket_sender(int tcp_port);
	int read_from_tcp_socket(char *buffer,int buffer_size);
	int write_to_tcp_socket(char *buffer,int buffer_size);
	void close_tcp_sender_socket();

};

void tcp_socket_sender::close_tcp_sender_socket()
{
	close(sockfd);
}


int tcp_socket_sender::write_to_tcp_socket(char *buffer,int buffer_size)
{
	int n = write(sockfd,buffer, buffer_size);
	if(n<0)
		perror("\nERROR writing to socket\n");
	return n;
}

int tcp_socket_sender::read_from_tcp_socket(char *buffer,int buffer_size)
{
	int n = read(sockfd, buffer, buffer_size);
	if (n < 0)
       	perror("ERROR reading from socket");
    return n;
}




tcp_socket_sender::tcp_socket_sender(int tcp_port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {		
		perror("ERROR opening socket");
	}	
	bzero((char*)&serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(tcp_port);
	
	if(connect(sockfd, (struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0)
    	perror("ERROR connecting");
}