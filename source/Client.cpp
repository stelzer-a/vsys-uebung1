#include "../header/Client.h"

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
	size_t comm_len = BUF * sizeof(char);
	char* comm = (char*) malloc(comm_len);

    // Es können solange Befehle eingegeben werden, bis "quit\n.\n" eingegeben wird
    do {
        // Eingegebenen Befehl auslesen 
		printf("Enter your command below:\n");

		// Befehl Zeile für Zeile auslesen, bis ".\n" eingegeben wird
		do {
				getline(&line, &comm_len, stdin);
				if ((strlen(line) + strlen(comm)) > (BUF - 2)) {
					printf("Command is too large!\n");
                    free(line);
                    free(comm);
					return;
				} else {
					comm = strncat(comm, line, strlen(line) + 1);
				}
		} while (strcmp(line, ".\n"));

		// Zu Debugzwecken comm ausgeben
		printf("%s\n", comm);

        // Befehlsstring zurücksetzen
        if (strcmp(comm, "quit\n.\n")) {
			// comm-String zurücksetzen
			memset(comm, 0, strlen(comm));
		}

    } while (strncmp(comm, "quit", 4));

    // Allozierten Speicher freigeben
	free(line);
	free(comm);
}

// char* receive(Buffer)
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