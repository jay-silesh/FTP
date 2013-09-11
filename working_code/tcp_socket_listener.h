class tcp_socket_listener{
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	
public:
	tcp_socket_listener(int tcp_port);
	int read_from_tcp_socket(char *buffer,int buffer_size);
	int write_to_tcp_socket(char *buffer,int buffer_size);
	void close_tcp_listener_socket();
	void newclose_tcp_listener_socket();
	struct sockaddr_in get_sockaddr();	

};

struct sockaddr_in tcp_socket_listener::get_sockaddr()
{
	return cli_addr;
}

void tcp_socket_listener::close_tcp_listener_socket()
{
	close(sockfd);
}

void tcp_socket_listener::newclose_tcp_listener_socket()
{
	close(newsockfd);
}
int tcp_socket_listener::write_to_tcp_socket(char *buffer,int buffer_size)
{
	int n = write(newsockfd,buffer, buffer_size);
	if(n<0)
		perror("\nERROR writing to socket\n");
	return n;
}

int tcp_socket_listener::read_from_tcp_socket(char *buffer,int buffer_size)
{
	int n = read(newsockfd, buffer, buffer_size);
	if (n < 0)
       	perror("ERROR reading from socket");
    return n;
}

tcp_socket_listener::tcp_socket_listener(int tcp_port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {		
		perror("ERROR opening socket");
	}	
	bzero((char*)&serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(tcp_port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		perror("ERROR on binding");
	listen(sockfd,10);
	clilen = sizeof(cli_addr); 
	
	newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,&clilen);
	if (newsockfd < 0)
		perror("ERROR on accept");
}


