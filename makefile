task:
		rm -f *.o

		g++ -c Packet.cc -o Packet
		g++ -g client.cc Packet -o client -pthread
		g++ -g server.cc Packet -o server -pthread
		
clean:
		rm -f *.o
		rm output.txt
