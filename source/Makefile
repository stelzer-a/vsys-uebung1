all: twmailerClient twmailerServer

clean:
	rm twmailerServer
	rm twmailerServer.o
	rm twmailerClient
	rm twmailerClient.o
	rm Server.o
	rm Client.o

Client.o: Client.cpp
	g++ -c Client.cpp

Server.o: Server.cpp
	g++ -c Server.cpp

twmailerClient.o: twmailerClient.cpp
	g++ -c twmailerClient.cpp

twmailerServer.o: twmailerServer.cpp
	g++ -c twmailerServer.cpp


twmailerClient: twmailerClient.o Client.o
	g++ -o twmailerClient twmailerClient.o Client.o

twmailerServer: twmailerServer.o Server.o
	g++ -o twmailerServer twmailerServer.o Server.o

