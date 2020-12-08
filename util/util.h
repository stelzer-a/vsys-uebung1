#ifndef TWMAILER_UTIL
#define TWMAILER_UTIL

#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

#define HEADER_BYTES 4
#define MAX_USERID_SIZE 8
#define MAX_SUBJECT_SIZE 80
#define OK_STRING "OK\n"
#define ERR_STRING "ERR\n"

void recv_all(int socket, char* buffer, int nBytes);
void send_all(int socket, char* msg, int nBytes);

const char* getpass();

#endif