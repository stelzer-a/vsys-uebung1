#include "../header/Server.h"

// Konstruktor
// initialisiert Socket-Deskriptor, Maildirectory-String, Adresslänge eines Sockets und Input-Buffer
// 
Server::Server() {
    listen_socket = -1;
    maildir = NULL;
    sock_addr_len = sizeof(struct sockaddr_in);
    recv_buffer = (char*) malloc(BUF * sizeof(char));
    buffer_size = BUF;
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
            send_msg("Willkommen bei twmailer!\n\n");
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

// void send_msg(char* msg)
// Sendet den String msg an den Client
// gibt -1 zurück, wenn send() einen Fehler zurückgibt, ansonsten Anzahl der gesendeten Bytes
// 
void Server::send_msg(char* msg) {
    // Die übergebene Nachricht an den Client schicken
    send_all(client_socket, msg, strlen(msg));
}

// int recv_cmd()
// Liest in einer Schleife die Nachrichten vom Client aus
// 
void Server::recv_cmd() {
    int msg_size;
    do {
        // Zuerst die Nachrichtenlänge in Bytes auslesen (32 Bit Zahl --> 4 Bytes)
        recv_all(client_socket, recv_buffer, HEADER_BYTES);
        msg_size = atoi(recv_buffer);

        // Prüfen, ob recv_buffer groß genug ist für den einzulesenden String
        if (msg_size >= buffer_size) {
            buffer_size = (msg_size/(BUF) + 1) * BUF;
            recv_buffer = (char *) realloc(recv_buffer, buffer_size * sizeof(char));
        }

        if (msg_size > 0) {
            // Danach restliche Nachricht einlesen
            recv_all(client_socket, recv_buffer, msg_size);

            // Eingelesenen String an cmd_string übergeben, da recv_buffer nicht von den folgenden Stringfunktionen
            // manipuliert werden darf, da sonst realloc() später nicht mehr funktioniert
            cmd_string = recv_buffer;

            // Eingelesenen Befehlsstring parsen
            if (parseCmd() == -1) {
                // Nachricht ERR an Client senden
                send_all(client_socket, ERR_STRING, strlen(ERR_STRING));

                // cmd_string wieder auf recv_buffer setzen, das es sonst beim
                // strncasecmp() zu einem Segmentation Fault kommen könnte wenn cmd_string NULL ist
                cmd_string = recv_buffer;
            } else {
                // Nachricht OK an Client senden
                send_all(client_socket, OK_STRING, strlen(OK_STRING));
            }
        } else if (msg_size == 0) {
           	printf("Client closed remote socket\n");
           	break;
        } else {
            printf("Error receiving message\n");
        } 

    } while (strncasecmp(cmd_string, "quit", 4) != 0 || msg_size == 0);
}

// int parseCmd()
// Liest den Befehl aus dem empfangenen String aus
// gibt -1 zurück, wenn ein Fehler aufgetreten ist 
//
int Server::parseCmd() {
    // String bei Newline spalten und Befehl auslesen
    char* cmd = strsep(&cmd_string, "\n");

    // Je nach eingegebenem Befehl in andere Funktionen verzweigen
    if (strcasecmp(cmd, "send") == 0) {
        if (handleSend() == -1) {
            printf("Error handling SEND command!\n");
            return -1;
        }
        return 0;
    } else if (strcasecmp(cmd, "list") == 0) {
        if (handleList() == -1) {
            printf("Error handling LIST command!\n");
            return -1;
        }
    } else if (strcasecmp(cmd, "read") == 0) {
        printf("Command %s is valid!\n", cmd);
        return 0;
    } else if (strcasecmp(cmd, "del") == 0) {
        printf("Command %s is valid!\n", cmd);
        return 0;
    } else if (strcasecmp(cmd, "quit") == 0) {
        // recv_buffer wieder auf "quit" setzen, um Schleife in recv_cmd() beenden zu können
        strcpy(cmd_string, "quit");
        return 0;
    } else {
        printf("Command %s is invalid!\n", cmd);
        return -1;
    }

    return 0;
}

// int handleSend()
// Kümmert sich um die Verarbeitung des Befehlsstrings, wenn es ein SEND-Befehl ist
// gibt -1 zurück, wenn ein Fehler aufgetreten ist
//
int Server::handleSend() {
    // Absender auslesen und auf Gültigkeit überprüfen
    char* sender = strsep(&cmd_string, "\n");
    if(cmd_string == NULL) {
        printf("SEND command is invalid!\n");
        return -1;
    } else if (sender == "\0" || strlen(sender) > MAX_USERID_SIZE) {
        printf("Sender %s is invalid!\n", sender);
        return -1;
    }

    // Empfänger auslesen und auf Gültigkeit überprüfen
    char* recipient = strsep(&cmd_string, "\n");
    if(cmd_string == NULL) {
        printf("SEND command is invalid!\n");
        return -1;
    } else if (recipient == "\0" || strlen(recipient) > MAX_USERID_SIZE) {
        printf("Recipient %s is invalid!\n", recipient);
        return -1;
    }

    // Betreff auslesen und auf Gültigkeit überprüfen
    char* subject = strsep(&cmd_string, "\n");
    if(cmd_string == NULL) {
        printf("SEND command is invalid!\n");
        return -1;
    } else if (subject == "\0" || strlen(subject) > MAX_SUBJECT_SIZE) {
        printf("Subject %s is invalid!\n", subject);
        return -1;
    }

    // Der restliche String (ohne ".\n") ist der Inhalt der Mail
    if (strlen(cmd_string) > 0) {
        char* content = (char* ) malloc((strlen(cmd_string) - 1) * sizeof(char));
        strncpy(content, cmd_string, strlen(cmd_string) - 2);
        content[strlen(cmd_string) - 2] = '\0';

        // Mail-Objekt erstellen und speichern
        Mail newMail(sender, recipient, subject, content);
        newMail.displayValues();
        newMail.saveToMaildir(maildir);

        free(content);
        return 0;
    } else {
        printf("SEND command is invalid!\n");
        return -1;
    }
    
}

// int handleList()
// Kümmert sich um die Verarbeitung des Befehlsstrings, wenn es ein LIST-Befehl ist
// gibt -1 zurück, wenn ein Fehler aufgetreten ist
//
int Server::handleList() {
    int mails;

    if(cmd_string == NULL) {
        printf("LIST command is invalid!\n");
        return -1;
    } else {
        // Pfadstring für das Mailverzeichnis des Users erzeugen
        char* user = strsep(&cmd_string, "\n");
        char* user_dir_string = (char*) malloc((strlen(maildir) + strlen(user) + 2) * sizeof(char));
        sprintf(user_dir_string, "%s/%s", maildir, user);

        // Verzeichnis durchsuchen und Anzahl der Mails angeben
        DIR* user_dir = opendir(user_dir_string);
        if (errno == ENOENT) {
            // Das Verzeichnis gibt es noch gar nicht --> noch keine Mails für diesen User
            mails = 0;
        } else if (user_dir) {
            // In einer Schleife die Anzahl an Mails lesen
            struct dirent *dirp;
            do {
                // Files im Verzeichnis lesen und (falls es ein File ist) zählen
                dirp = readdir(user_dir);
                if (dirp->d_type == DT_REG || dirp->d_type == DT_UNKNOWN) {
                    mails++;
                }
            } while (dirp != NULL);
        } else {
            // Fehler
            return -1;
        }
    }
    
    return 0;
}