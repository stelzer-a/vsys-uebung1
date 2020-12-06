#include "../header/Client.h"

// Usage-Fehlermeldung
void print_usage(char* program_name) {
	fprintf(stderr, "Usage: %s [-p  Port] [-i IP-Address]\n", program_name);
	exit(1);
}

int main (int argc, char *argv[]) {
	int server_port;
	char* server_addr;

	// Prüfen, ob die richtige Anzahl an Kommandozeilenparametern eingegeben wurde
	if(argc != 5) {
		print_usage(argv[0]);
	}

	// Lesen der Kommandozeilenparameter
	int c;
	int pOption = 0;
	int iOption = 0;
	while((c = getopt(argc, argv, "p:i:")) != EOF) {
		switch(c) {
			case 'p':
				pOption++;
				server_port = (int) strtol(optarg, NULL, 0);
				if (server_port <= 0 || server_port > 65535) {
					printf("Please choose a valid port!\n");
					return EXIT_FAILURE;				
				} 
				break;
			case 'i':
				iOption++;
				server_addr = optarg;
				break;
			case '?':
				print_usage(argv[0]);
			default:
				return EXIT_FAILURE;
		}
	}

	// Prüfen, ob ein Argument öfter vorgekommen ist
	if(pOption > 1 || iOption > 1) {
		print_usage(argv[0]);
	}

	// Client erstellen und initialisieren
	Client client;
	if ((client.init(server_addr, server_port)) == -1) {
		perror("Connect error - socket could not be created");
     	return EXIT_FAILURE;
	}

	// Mit Server verbinden und Willkommensnachricht empfangen
	if ((client.connectToServer()) == -1) {
		perror("Connect error - no server available");
     	return EXIT_FAILURE;
	} 
	
	// Kommandoeingabe-Schleife beginnen
	client.send_cmd();

	// Verbindung zum Server trennen
	client.disconnect();

	return EXIT_SUCCESS;
}
