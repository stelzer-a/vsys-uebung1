#include "../header/Server.h"

// Usage-Fehlermeldung
void print_usage(char* program_name) {
	fprintf(stderr, "Usage: %s [-p  Port] [-d mailspooldirectory]\n", program_name);
	exit(1);
}

int main (int argc, char* argv[]) {
	// Variablen - Kommandozeile
	int server_port;
	char* maildir_arg;
	DIR* maildir;	

	// Prüfen, ob die richtige Anzahl an Kommandozeilenparametern eingegeben wurde
	if(argc != 5) {
		print_usage(argv[0]);
	}

	// Lesen der Kommandozeilenparameter
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

  	// Prüfen, ob ein Argument öfter vorgekommen ist
	if(pOption > 1 || dOption > 1) {
		print_usage(argv[0]);
	}

	// Prüfen, ob das Mailspoolverzeichnis bereits existiert, erstellt werden muss,
	// oder eine falsche Eingabe passiert ist
	maildir = opendir(maildir_arg);
	if (maildir) {
		// Verzeichnis existiert
		printf("Accessing the mailspooldirectory...\n");
		closedir(maildir);
	} else if (errno == ENOENT) {
		// Verzeichnis existiert nicht, also erstellen
		printf("Creating the mailspooldirectory...\n");
		if (mkdir(maildir_arg, 0777) == 0) {
			printf("Mailspooldirectory created.\n");
		} else {
			perror("Error creating mailspooldirectory.\n");
			return EXIT_FAILURE;
		}
	} else {
		// Anderer Fehler ist aufgetreten
		perror("Error accessing mailspooldirectory.\n");
		return EXIT_FAILURE;
	}
	
	Server server;
	server.init(server_port, maildir_arg);

	//---------------- LDAP START ---------------- 

	//LDAP_load_Creds
	if (server.LDPA_load_Creds() == 0)
	{
		printf("LDAP Credentials load successfully!\n");
	}
	else{ printf("faild to load LDAP Creds");}

	//---------------- LDAP END ---------------- 

	server.start();
	server.stop();
	
  	return EXIT_SUCCESS;
}
