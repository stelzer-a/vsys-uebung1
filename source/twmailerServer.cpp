#include "../header/Server.h"

/* Usage-Fehlermeldung */
void print_usage(char* program_name) {
	fprintf(stderr, "Usage: %s [-p  Port] [-m mailspooldirectory]\n", program_name);
	exit(1);
}

int main (int argc, char* argv[]) {
	/* Variablen - Kommandozeile */
	int server_port;
	char* maildir_arg;
	DIR* maildir;

	/* Variablen - Server Socket(s) 	
	int listen_socket, new_socket;
 	socklen_t addrlen;
  	char buffer[BUF];
  	int size;
  	struct sockaddr_in server_address;
	
	/* Variablen - Client Socket(s) 
	struct sockaddr_in client_address; */	

	/* Prüfen, ob die richtige Anzahl an Kommandozeilenparametern eingegeben wurde */
	if(argc != 5) {
		print_usage(argv[0]);
	}

	/* Lesen der Kommandozeilenparameter */
	int c;
	int pOption = 0;
	int dOption = 0;
	while((c = getopt(argc, argv, "p:d:")) != EOF) {
		switch(c) {
			case 'p':
				pOption++;
				server_port = (int) strtol(optarg, NULL, 0);
				if (server_port <= 1023 || server_port > 65535) {
					printf("please choose a port between 1024 and 65535!\n");
					return EXIT_FAILURE;				
				} 
				break;
			case 'd':
				dOption++;
				maildir_arg = optarg;
				break;
			case '?':
				print_usage(argv[0]);
			default:
				return EXIT_FAILURE;
		}
	}

  	/* Prüfen, ob ein Argument öfter vorgekommen ist */
	if(pOption > 1 || dOption > 1) {
		print_usage(argv[0]);
	}

	/* Prüfen, ob das Mailspoolverzeichnis bereits existiert, erstellt werden muss,
	   oder eine falsche Eingabe passiert ist */
	maildir = opendir(maildir_arg);
	if (maildir) {
		/* Verzeichnis existiert */
		printf("Accessing the mailspooldirectory...\n");
		closedir(maildir);
	} else if (errno == ENOENT) {
		/* Verzeichnis existiert nicht, also erstellen */
		printf("Creating the mailspooldirectory...\n");
		if (mkdir(maildir_arg, 770) == 0) {
			printf("Mailspooldirectory created.\n");
		} else {
			perror("Error creating mailspooldirectory.\n");
			return EXIT_FAILURE;
		}
	} else {
		/* Anderer Fehler ist aufgetreten */
		perror("Error accessing mailspooldirectory.\n");
		return EXIT_FAILURE;
	}

  	/* Listener-Socket mit TCP erzeugen
  	listen_socket = socket(AF_INET, SOCK_STREAM, 0);

  	// Adresse des Servers auf eigene IP setzen
  	memset(&server_address,0,sizeof(server_address));
  	server_address.sin_family = AF_INET;
  	server_address.sin_addr.s_addr = INADDR_ANY;
  	server_address.sin_port = htons(server_port); 

  	// Die eigene IP wird dem Listener-Socket zugewiesen
  	if (bind(listen_socket, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
     		perror("Bind error");
     		return EXIT_FAILURE;
  	} */
	
	Server server;
	server.init(server_port, maildir_arg);

  	/* Auf Verbindungsanfragen von Clients warten 
  	listen(listen_socket, 0); 

  	addrlen = sizeof(struct sockaddr_in); */

	server.start();

  	/*while (1) {
     		printf("Waiting for connection on port %d...\n", server_port);

     		/* Ein Client hat eine Anfrage an den Listen-Socket geschickt 
     		new_socket = accept(listen_socket, (struct sockaddr *) &client_address, &addrlen );

     		/* Willkommen-Nachricht an den Client schicken 
     		if (new_socket > 0) {
        		printf ("Client connected from %s:%d...\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        		strcpy(buffer,"Welcome to twmailer\n");
        		send(new_socket, buffer, strlen(buffer), 0);
     		}

		/* Nachrichten vom Client empfangen 
     		do {
        		size = recv(new_socket, buffer, BUF-1, 0);
        		if (size > 0) {
           			buffer[size] = '\0';
           			printf ("Message received: %s\n", buffer);
        		}
        		else if (size == 0) {
           			printf("Client closed remote socket\n");
           			break;
        		}
        		else {
           			perror("recv error");
           			return EXIT_FAILURE;
        		}
     		} while (strncmp(buffer, "quit", 4)  != 0);
     		close(new_socket);
  	} 
  	close(listen_socket); */
	server.stop();
  	return EXIT_SUCCESS;
}
