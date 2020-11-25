#include "../header/Server.h"

// Konstruktor
// initialisiert Socket-Deskriptor, Maildirectory-String, Adresslänge eines Sockets und Input-Buffer
// 
Server::Server() {
    listen_socket = -1;
    maildir = NULL;
    sock_addr_len = sizeof(struct sockaddr_in);
    recv_buffer = (char*) malloc(BUF * sizeof(char));
}

// Destruktor
// gibt den im Konstruktor allozierten Speicher wieder frei
// 
Server::~Server() {
    free(recv_buffer);
}

// int init(Server-Port, Mailspooldirectory)
// erstellt einen Listen-Socket, initialisiert die Server-Addressen-Struktur und bindet den Socket an die Adresse
// gibt -1 zurück, wenn der Socket nicht erzeugt werden kann
// 
int Server::init(int srv_port, char* mail_dir) {
    // TCP-Socket erstellen
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    // Server-Addresse befüllen
  	memset(&srv_addr_struct, 0, sizeof(srv_addr_struct));
  	srv_addr_struct.sin_family = AF_INET;
    srv_addr_struct.sin_addr.s_addr = INADDR_ANY;
  	srv_addr_struct.sin_port = htons(srv_port);
    
    // Socket an Serveradresse binden
    if (bind(listen_socket, (struct sockaddr *) &srv_addr_struct, sizeof(srv_addr_struct)) == -1) {
     		return -1;
  	}
    
    maildir = mail_dir;

    return listen_socket;
}

// int start()
// listen() und Schleife, in der neue Clients akzeptiert werden
// gibt -1 zurück, wenn listen() einen Fehler liefert
// 
int Server::start() {
    // Auf Verbindungsanfragen von Clients hören
    if (listen(listen_socket, 0) == -1) {
        return -1;
    }

    // Hauptschleife, in der neue Verbindungen aufgebaut werden, starten
    while (1) {
        printf("Waiting for connection on port %d...\n", ntohs(srv_addr_struct.sin_port));

        // Neue Verbindungsanfragen akzeptieren
        if (acceptClient() == -1) {
            printf("A client could not connect to the server\n");
        } else {
            // Willkommensnachricht an den Client schicken
            printf("Client connected from %s:%d\n", inet_ntoa(clt_addr_struct.sin_addr), ntohs(clt_addr_struct.sin_port));
            if ((send_msg("Willkommen bei twmailer!\n\n")) == -1) {
                printf("Error sending welcome message to the client\n");
            }
        }

        // Nachrichten vom Client empfangen bis die Verbindung getrennt wird
        recv_cmd();

        close(client_socket);
    }

    return 0;
}

// void stop()
// Schließt den Listener-Socket
// 
void Server::stop() {
    close(listen_socket);
}

// int acceptClient()
// accept() aus der Socket-Library
// gibt -1 zurück, wenn accept() einen Fehler zurückgibt
// 
int Server::acceptClient() {
    // Verbindungsanfrage akzeptieren und Client-Adress-Struktur befüllen
    client_socket = accept(listen_socket, (struct sockaddr *) &clt_addr_struct, &sock_addr_len);

    return client_socket;
}

// int send_msg(char* msg)
// Sendet den String msg an den Client
// gibt -1 zurück, wenn send() einen Fehler zurückgibt
// 
int Server::send_msg(char* msg) {
    // Die übergebene Nachricht an den Client schicken
    return send(client_socket, msg, strlen(msg), 0);
}

// int recv_cmd()
// Liest in einer Schleife die Nachrichten vom Client aus
// 
void Server::recv_cmd() {
    do {
        // Nachricht aus dem Socket lesen
        int msg_size;
        msg_size = recv(client_socket, recv_buffer, strlen(recv_buffer), 0);

        if (msg_size > 0) {
            recv_buffer[msg_size] = '\0';
            printf("Message received: %s\n", recv_buffer);
        } else if (msg_size == 0) {
           	printf("Client closed remote socket\n");
           	break;
        } else {
            printf("Error receiving message\n");
        }
    } while (strncmp(recv_buffer, "quit", 4));
}