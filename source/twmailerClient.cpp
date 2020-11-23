#include "../header/twmailerClient.h"

int main (int argc, char *argv[]) {
  	int create_socket;
  	char buffer[BUF];
  	struct sockaddr_in address;
  	int size;
	int server_port;
	char* server_addr;

	/* Prüfen, ob die richtige Anzahl an Kommandozeilenparametern eingegeben wurde */
	if(argc != 5) {
		print_usage(argv[0]);
	}

	/* Lesen der Kommandozeilenparameter */
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

	/* Prüfen, ob ein Argument öfter vorgekommen ist */
	if(pOption > 1 || iOption > 1) {
		print_usage(argv[0]);
	}

	/* TCP-Socket erstellen */
  	if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket error");
     		return EXIT_FAILURE;
	}

	/* Server-Addresse befüllen */
  	memset(&address, 0, sizeof(address));
  	address.sin_family = AF_INET;
  	address.sin_port = htons(server_port);
  	inet_aton(server_addr, &(address.sin_addr));

	/* Mit Server verbinden */
  	if (connect(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0) {
     		printf("Connection with server (%s) established\n", inet_ntoa(address.sin_addr));
     		size = recv(create_socket, buffer, BUF-1, 0);
     
		if (size > 0) {
        		buffer[size]= '\0';
        		printf("%s", buffer);
     		}
  	} else {
     		perror("Connect error - no server available");
     		return EXIT_FAILURE;
  	}

	/* Schleife für Senden von Befehlen an den Server */
  	do {
     		/*printf("Send message: ");
     		fgets(buffer, BUF, stdin);
     		send(create_socket, buffer, strlen (buffer), 0);*/
		send_cmd(buffer, BUF-1);
		printf("%s", buffer);
  	} while(strcmp(buffer, "quit\n") != 0);
  
	/* Socket schließen */
	close (create_socket);

	return EXIT_SUCCESS;
}
