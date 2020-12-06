#include "../header/Client.h"

// Konstruktor
// initialisiert Socket-Deskriptor und receive-Buffer
// 
Client::Client() {
    client_socket = -1;
    recv_buffer = (char*) malloc(BUF * sizeof(char));
	userid = NULL;
}

// Destruktor
// gibt allozierten Speicher wieder frei
// 
Client::~Client() {
    if (recv_buffer) {
        free(recv_buffer);
    }
	if (userid) {
        free(userid);
    }
}

// int init(Server-Addresse, Server-Port)
// erstellt einen Socket und initialisiert die Server-Addressen-Struktur
// gibt -1 zurück, wenn der Socket nicht erzeugt werden kann
// 
int Client::init(char* srv_addr, int srv_port) {
    // TCP-Socket erstellen
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Server-Addresse befüllen
  	memset(&srv_addr_struct, 0, sizeof(srv_addr_struct));
  	srv_addr_struct.sin_family = AF_INET;
  	srv_addr_struct.sin_port = htons(srv_port);
  	inet_aton(srv_addr, &(srv_addr_struct.sin_addr));

    return client_socket;
}

// int connectToServer()
// verbindet sich zum Server
// gibt -1 zurück, wenn die Verbindung nicht hergestellt werden kann
// 
int Client::connectToServer() {
    /* Mit Server verbinden */
  	return connect(client_socket, (struct sockaddr *) &srv_addr_struct, sizeof(srv_addr_struct));
}

// int disconnect()
// Schließt den Socket
// gibt -1 zurück, wenn die Verbindung nicht getrennt werden kann
// 
int Client::disconnect() {
    // Socket schließen
  	return close(client_socket);
}

// void send_cmd()
// Startet eine Schleife innerhalb der Befehle von der Konsole ausgelesen und
// an den Server geschickt werden
// 
void Client::send_cmd() {
    // lineptr initialisieren 	
	char* line = NULL;
	size_t cmd_len = BUF * sizeof(char);
	char* cmd = (char*) malloc(cmd_len);
	char* cmd_input = (char*) malloc(cmd_len);
	char* msg_length = (char*) malloc(sizeof(uint32_t));

    // Es können solange Befehle eingegeben werden, bis "quit\n.\n" eingegeben wird
    while (strcasecmp(cmd_input, "quit\n")) {
        // Eingegebenen Befehl auslesen 
		printf("\nEnter your command below:\n");

		// Befehlszeile auslesen und auf Gültigkeit überprüfen
		getline(&cmd, &cmd_len, stdin);
		cmd_input = (char*) realloc(cmd_input, (strlen(cmd) + 1) * sizeof(char));
		strcpy(cmd_input, cmd);

		/* Prüfen, ob sich der User bereits eingeloggt hat
		if(userid == NULL && strcasecmp(cmd, "login\n") != 0) {
			printf("\nYou are not logged in yet!\n");
		} else { */
			if (strcasecmp(cmd, "send\n") != 0 && strcasecmp(cmd, "list\n") != 0 && strcasecmp(cmd, "read\n") != 0 &&
				strcasecmp(cmd, "del\n") != 0 && strcasecmp(cmd, "quit\n") != 0 && strcasecmp(cmd, "login\n") != 0) {

				printf("%s ist not a valid command!\n", cmd);
			} else {

				// Userid an Befehl anhängen
				// falls der User sich noch nicht angemeldet hat wird ein 
				// String angehängt, der länger als 8 Zeichen ist, damit der Server es als Fehler erkennt
				if (strcasecmp(cmd, "login\n") != 0) {
					if (userid == NULL) {
						strcat(cmd, ERR_USER);
					} else {
						strcat(cmd, userid);
					}
				}

				readCmd(cmd);

				// Befehlsstring nullterminieren
				cmd[strlen(cmd)+1] = '\0';

				// Länge des Befehlsstrings an den Server senden
				uint32_t msg_size = strlen(cmd);
				sprintf(msg_length, "%d", msg_size);
				send_all(client_socket, msg_length, sizeof(uint32_t));

				// Befehlsstring an den Server senden
				send_all(client_socket, cmd, strlen(cmd));

				// Antwort vom Server empfangen
				// 2 bei READ und LIST ansonsten nur einmal
				if (strcasecmp(cmd_input, "read\n") == 0 || strcasecmp(cmd_input, "list\n") == 0) {
					receive(cmd_input);
				}
				receive(cmd_input);


				// Befehlsstring zurücksetzen
				if (strncasecmp(cmd, "quit", 4) != 0) {
					memset(cmd, 0, strlen(cmd));
				}
			}
		//}

    }

    // Allozierten Speicher freigeben
	free(line);
	free(cmd);
	free(msg_length);
	free(cmd_input);
}

// int receive(Befehl)
// Liest eine Nachricht aus dem Socket aus und gibt sie auf der Konsole aus
// gibt -1 zurück, wenn keine Nachricht empfangen werden kann 
// 
int Client::receive(char* cmd) {
    int size;

	// Zuerst die Nachrichtenlänge in Bytes auslesen (32 Bit Zahl --> 4 Bytes)
    recv_all(client_socket, recv_buffer, HEADER_BYTES);
    size = atoi(recv_buffer);

    // Nachricht vom Socket auslesen
    recv_all(client_socket, recv_buffer, size);

    if (size > 0) {
        recv_buffer[size]= '\0';
		printf("%s", recv_buffer);

		if (strcasecmp(recv_buffer, ERR_STRING) == 0 && strcasecmp(cmd, "login\n") == 0) {
			free(userid);
			userid == NULL;
		}
        return 0;
  	} else {
     	return -1;
  	}
}

// int readCmd(Buffer)
// Liest den vollständigen Befehl von der Konsole aus
// gibt -1 zurück, wenn ein Fehler auftritt
// 
int Client::readCmd(char* cmd) {
	char* line = NULL;
	size_t line_len;

    // Je nach Befehl wird das Lesen von der Konsole anders beendet
	if (strncasecmp(cmd, "send\n", 5) == 0) {
		// Befehl Zeile für Zeile auslesen bis ein ".\n" kommt
		do {
			getline(&line, &line_len, stdin);
			if ((strlen(line) + strlen(cmd)) > (BUF - 2)) {
				printf("Command is too large!\n");
				free(line);
				return -1;
			} else {
				cmd = strncat(cmd, line, strlen(line) + 1);
			}
		} while (strcmp(line, ".\n") != 0);
	} else if (strcasecmp(cmd, "login\n") == 0) {
		// Zuerst userid abfragen
		userid = (char*) malloc(BUF/8);
		size_t userid_len = BUF/8 - 1;
		printf("User: ");
		getline(&userid, &userid_len, stdin);
		strcat(cmd, userid);

		// Dann Passwort abfragen
		strcpy(pw, getpass());
		strcat(cmd, pw);
		cmd[strlen(cmd)] = '\n';
		cmd[strlen(cmd)] = '\0';

	} else {
		int count_lines;

		// Je nach Befehl bestimmte Anzahl an Zeilen noch auslesen
		if (strncasecmp(cmd, "list\n", 5) == 0 || strncasecmp(cmd, "quit\n", 5) == 0) {
			count_lines = 0;
		} else if (strcasecmp(cmd, "login\n") == 0) {
			count_lines = 2;
		} else {
			count_lines = 1;
		}

		while (count_lines) {
			getline(&line, &line_len, stdin);
			if ((strlen(line) + strlen(cmd)) > (BUF - 2)) {
				printf("Command is too large!\n");
				free(line);
				return -1;
			} else {
				cmd = strncat(cmd, line, strlen(line) + 1);
				count_lines--;
			}
		}
	}

	if(line) {
		free(line);
	}
	return 0;
}