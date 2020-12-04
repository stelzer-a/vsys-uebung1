all: twmailerClient twmailerServer

clean:
	rm twmailerServer
	rm twmailerServer.o
	rm twmailerClient
	rm twmailerClient.o
	rm Server.o
	rm Client.o
	rm Mail.o
	rm util.o
	rm mypw.o

Client.o: Client.cpp
	g++ -c Client.cpp

Server.o: Server.cpp
	g++ -c Server.cpp
	
Mail.o: Mail.cpp
	g++ -c Mail.cpp

util.o: util.cpp
	g++ -c util.cpp

mypw.o: mypw.cpp
	g++ -c mypw.cpp

twmailerClient.o: twmailerClient.cpp
	g++ -c twmailerClient.cpp

twmailerServer.o: twmailerServer.cpp
	g++ -c twmailerServer.cpp


twmailerClient: twmailerClient.o Client.o util.o mypw.o
	g++ -o twmailerClient twmailerClient.o Client.o util.o mypw.o

twmailerServer: twmailerServer.o Server.o Mail.o util.o
	g++ -o twmailerServer twmailerServer.o Server.o Mail.o util.o

