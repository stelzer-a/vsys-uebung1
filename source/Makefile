CC=g++
CFLAGS=-g -Wall -Wextra -O -std=c++2a -pthread
LIBS=-lldap -llber -luuid

rebuild: clean all
all: twmailerClient twmailerServer ldapclient

clean:
	rm twmailerServer
	rm twmailerServer.o
	rm twmailerClient
	rm twmailerClient.o
	rm Server.o
	rm Client.o
	rm mypw.o
	rm ldapclient.o
	rm ldapclient
	

Client.o: Client.cpp
	${CC} ${CFLAGS} -o Client.o Client.cpp -c

Server.o: Server.cpp
	${CC} ${CFLAGS} -o Server.o Server.cpp -c

twmailerClient.o: twmailerClient.cpp
	${CC} ${CFLAGS} -o twmailerClient.o twmailerClient.cpp -c

twmailerServer.o: twmailerServer.cpp
	${CC} ${CFLAGS} -o twmailerServer.o twmailerServer.cpp -c

ldapclient.o: myldap.cpp
	${CC} ${CFLAGS} -o ldapclient.o myldap.cpp -c

mypw.o: mypw.cpp
	${CC} ${CFLAGS} -o mypw.o mypw.cpp -c

twmailerClient: twmailerClient.o Client.o
	${CC} ${CFLAGS} -o twmailerClient twmailerClient.o Client.o

twmailerServer: twmailerServer.o Server.o
	${CC} ${CFLAGS} -o twmailerServer twmailerServer.o Server.o mypw.o ${LIBS}

ldapclient: ldapclient.o mypw.o
	${CC} ${CFLAGS} -o ldapclient mypw.o ldapclient.o ${LIBS}