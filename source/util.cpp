#include "../util/util.h"

void recv_all(int socket, char* buffer, int nBytes) {
    int bytesRead;
    int totalRead = 0;
    do {
        bytesRead = recv(socket, buffer+totalRead, nBytes, 0);
        nBytes -= bytesRead;
        totalRead += bytesRead;
    } while (nBytes > 0);
    buffer[totalRead] = '\0';
}

void send_all(int socket, char* msg, int nBytes) {
    int bytesSent;
    int totalSent = 0;
    do {
        bytesSent = send(socket, msg+totalSent, nBytes, 0);
        nBytes -= bytesSent;
        totalSent += bytesSent;
    } while (nBytes > 0);
}