#include "../header/Client.h"
#include "../util/mypw.h"

// Konstruktor
// initialisiert Socket-Deskriptor und receive-Buffer
// 
Client::Client() {
    client_socket = -1;
    recv_buffer = (char*) malloc(BUF * sizeof(char));
}

// Destruktor
// gibt den für receive-Buffer allozierten Speicher wieder frei
// 
Client::~Client() {
    if (recv_buffer != NULL) {
        free(recv_buffer);
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
	char* msg_length = (char*) malloc(sizeof(uint32_t));
	bool end_read = false;

    // Es können solange Befehle eingegeben werden, bis "quit\n.\n" eingegeben wird
    do {
        // Eingegebenen Befehl auslesen 
		printf("\nEnter your command below:\n");

		// Befehlszeile auslesen und auf Gültigkeit überprüfen
		getline(&cmd, &cmd_len, stdin);

		if (strcasecmp(cmd, "send\n") != 0 && strcasecmp(cmd, "list\n") != 0 && strcasecmp(cmd, "read\n") != 0 &&
			strcasecmp(cmd, "del\n") != 0 && strcasecmp(cmd, "quit\n") != 0 && strcasecmp(cmd, "login\n") != 0) {

			printf("%s ist not a valid command!\n", cmd);
		} else {

			readCmd(cmd);

			// Befehlsstring nullterminieren
			cmd[strlen(cmd)+1] = '\0';

			// Länge des Befehlsstrings an den Server senden
			uint32_t msg_size = strlen(cmd);

			sprintf(msg_length, "%d", msg_size);

			// send(client_socket, msg_length, sizeof(uint32_t), 0);
			send_all(client_socket, msg_length, sizeof(uint32_t));

			// Befehlsstring an den Server senden
			// send(client_socket, cmd, strlen(cmd), 0);
			send_all(client_socket, cmd, strlen(cmd));

			// Antwort vom Server empfangen
			receive();

			// Befehlsstring zurücksetzen
			if (strncasecmp(cmd, "quit", 4) != 0) {
				// cmd-String zurücksetzen
				memset(cmd, 0, strlen(cmd));
			}
		}

    } while (strncmp(cmd, "quit", 4));

    // Allozierten Speicher freigeben
	free(line);
	free(cmd);
	free(msg_length);
}

// int receive()
// Liest eine Nachricht aus dem Socket aus und gibt sie auf der Konsole aus
// gibt -1 zurück, wenn keine Nachricht empfangen werden kann 
// 
int Client::receive() {
    int size;

    // Nachricht vom Socket auslesen
    size = recv(client_socket, recv_buffer, BUF - 1, 0);

    if (size > 0) {
        recv_buffer[size]= '\0';
        printf("%s", recv_buffer);
        return 0;
  	} else {
     	return -1;
  	}
}

// int receive(Buffer)
// Liest den vollständigen Befehl von der Konsole aus
// gibt -1 zurück, wenn ein Fehler auftritt
// 
int Client::readCmd(char* cmd) {
	char* line = NULL;
	size_t line_len;

    // Je nach Befehl wird das Lesen von der Konsole anders beendet
	if (strcasecmp(cmd, "send\n") == 0) {
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
		userid = (char*) malloc(128);
		pw = (char*) malloc(256);
		size_t userid_len = 127;
		getline(&userid, &userid_len, stdin);

		// Dann Passwort abfragen
		strcpy(pw, getpass());
		printf("User: %s - %s", userid, pw);

		free(userid);
		free(pw);
	} else {
		int count_lines;

		// Je nach Befehl bestimmte Anzahl an Zeilen noch auslesen
		if (strcasecmp(cmd, "list\n") == 0) {
			count_lines = 1;
		} else if (strcasecmp(cmd, "quit\n") == 0) {
			count_lines = 0;
		} else {
			count_lines = 2;
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

	if(line != NULL) {
		free(line);
	}
	return 0;
}