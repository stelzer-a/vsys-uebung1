CC=g++
CFLAGS=-g -O -std=c++2a -pthread
LIBS=-lldap -llber -luuid

rebuild: clean all
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

Client.o: Client.cpp
	${CC} ${CFLAGS} -o Client.o Client.cpp -c

Server.o: Server.cpp
	${CC} ${CFLAGS} -o Server.o Server.cpp -c

Mail.o: Mail.cpp
	${CC} ${CFLAGS} -o Mail.o Mail.cpp -c

util.o: util.cpp
	${CC} ${CFLAGS} -o util.o util.cpp -c

twmailerClient.o: twmailerClient.cpp
	${CC} ${CFLAGS} -o twmailerClient.o twmailerClient.cpp -c

twmailerServer.o: twmailerServer.cpp
	${CC} ${CFLAGS} -o twmailerServer.o twmailerServer.cpp -c

twmailerClient: twmailerClient.o Client.o util.o
	${CC} ${CFLAGS} -o twmailerClient twmailerClient.o Client.o util.o

twmailerServer: twmailerServer.o Server.o Mail.o util.o
	${CC} ${CFLAGS} -o twmailerServer twmailerServer.o Server.o Mail.o util.o ${LIBS}

