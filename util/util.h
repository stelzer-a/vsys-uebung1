#ifndef TWMAILER_UTIL
#define TWMAILER_UTIL

#include <sys/socket.h>

void recv_all(int socket, char* buffer, int nBytes);
void send_all(int socket, char* msg, int nBytes);

#endif