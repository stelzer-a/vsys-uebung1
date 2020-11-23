#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUF 1024

/* Usage-Fehlermeldung */
void print_usage(char* program_name) {
	fprintf(stderr, "Usage: %s [-p  Port] [-i IP-Address]\n", program_name);
	exit(1);
}

/* Funktion zum Senden von Befehlen */
void send_cmd(char* buffer, size_t buff_len) {
	/* lineptr initialisieren */	
	char* line = NULL;

	/* Eingegebene Zeile auslesen */
	printf("Enter your command below:\n");
	getline(&line, &buff_len, stdin);
	buffer = strcpy(buffer, line);	

	/* lineptr (von getline() alloziert) wieder freigeben */
	free(line);
}
