task:
		g++ client.cc -o client
		g++ server.cc -o server
		
clean:
		rm -f *.o
