#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUF 1024

/* Usage-Fehlermeldung */
void print_usage(char* program_name) {
	fprintf(stderr, "Usage: %s [-p  Port] [-m mailspooldirectory]\n", program_name);
	exit(1);
}
