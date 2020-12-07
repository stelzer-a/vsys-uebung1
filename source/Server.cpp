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
    if (listen(listen_socket, 5) == -1) {
        return -1;
    }

    // Hauptschleife, in der neue Verbindungen aufgebaut werden, starten
    while (1) {
        printf("Waiting for connection on port %d...\n", ntohs(srv_addr_struct.sin_port));

        // Neue Verbindungsanfragen akzeptieren
        if (acceptClient() == -1) {
            printf("A client could not connect to the server\n");
        } 

        printf("client connected..\n");

        // Nachrichten vom Client empfangen bis die Verbindung getrennt wird
        // recv_cmd();

        /*for (int i = 0; i < threads.size(); i++) {
            printf("hallo\n");
            threads[i].join();
            close(client_sockets[i]);
        }*/

        // close(client_socket);
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
    client_sockets.push_back(accept(listen_socket, (struct sockaddr *) &clt_addr_struct, &sock_addr_len));

    // Thread erstellen und starten
    std::thread newThread(&Server::recv_cmd, this, client_sockets.back());
    threads.push_back(std::move(newThread));

    return client_sockets.back();
}

// void send_msg(char* msg)
// Sendet den String msg an den Client
// gibt -1 zurück, wenn send() einen Fehler zurückgibt, ansonsten Anzahl der gesendeten Bytes
// 
void Server::send_msg(int client_socket, char* msg) {
    char* msg_length = (char*) malloc(sizeof(uint32_t));

    uint32_t msg_size = strlen(msg);
	sprintf(msg_length, "%d", msg_size);
	send_all(client_socket, msg_length, sizeof(uint32_t));

    // Die übergebene Nachricht an den Client schicken
    send_all(client_socket, msg, strlen(msg));

    free(msg_length);
}

// int recv_cmd()
// Liest in einer Schleife die Nachrichten vom Client aus
// 
void Server::recv_cmd(int client_socket) {
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
            if (parseCmd(client_socket) == -1) {
                // Nachricht ERR an Client senden
                send_msg(client_socket, ERR_STRING);

                // cmd_string wieder auf recv_buffer setzen, das es sonst beim
                // strncasecmp() zu einem Segmentation Fault kommen könnte wenn cmd_string NULL ist
                cmd_string = recv_buffer;
            } else {
                // Nachricht OK an Client senden
                send_msg(client_socket, OK_STRING);
            }
        } else if (msg_size == 0) {
           	printf("Client closed remote socket\n");
           	break;
        } else {
            printf("Error receiving message\n");
        } 

    } while (strncasecmp(cmd_string, "quit", 4) != 0 || msg_size == 0);

    // Socket schließen
    close(client_socket);
}


// int readSubject(Verzeichniseintrag, Zielstring)
// Liest den Betreff aus einem Mail aus
// gibt -1 zurück, wenn ein Fehler aufgetreten ist 
//
int Server::readSubject(struct dirent *dirp, char* path , char** subject) {
    // Dateipfad erzeugen
    char* file_path = (char*) malloc((strlen(path) + strlen(dirp->d_name) + 2) * sizeof(char));
    sprintf(file_path, "%s/%s", path, dirp->d_name);

    FILE* fptr;
    if ((fptr = fopen(file_path, "r")) == NULL) {
        printf("Error opening mail\n");
        free(file_path);
        return -1;
    }

    // Zeilen lesen bis Betreff eingelesen wurde
    size_t subject_len;
    ssize_t bytesRead;
    do {
        bytesRead = getline(subject, &subject_len, fptr);
    } while(strncasecmp(*subject, "Subject", strlen("Subject")) != 0 && bytesRead != -1);

    // Falls bis EOF kein Betreff eingelesen wurde
    if (bytesRead == -1) {
        free(file_path);
        printf("Error no subject\n");
        fclose(fptr);
        return -1;
    }

    free(file_path);
    fclose(fptr);
    return 0;
}

// int parseCmd()
// Liest den Befehl aus dem empfangenen String aus
// gibt -1 zurück, wenn ein Fehler aufgetreten ist 
//
int Server::parseCmd(int client_socket) {

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
        if (handleList(client_socket) == -1) {
            printf("Error handling LIST command!\n");
            return -1;
        }
        return 0;
    } else if (strcasecmp(cmd, "read") == 0) {
        if (handleRead(client_socket) == -1) {
            printf("Error handling READ command!\n");
            return -1;
        }
        return 0;
    } else if (strcasecmp(cmd, "del") == 0) {
        if (handleDel() == -1) {
            printf("Error handling DEL command!\n");
            return -1;
        }
        return 0;
    } else if (strcasecmp(cmd, "quit") == 0) {
        // recv_buffer wieder auf "quit" setzen, um Schleife in recv_cmd() beenden zu können
        strcpy(cmd_string, "quit");
        return 0;
    } else if (strcasecmp(cmd, "login") == 0) {
        if (handleLogin() == -1) {
            printf("Error handling LOGIN command!\n");
            return -1;
        }
        return 0;
    }  else {
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
int Server::handleList(int client_socket) {
    int mails = 0;
    char* user_dir_string = NULL;
    char* number_mails_string = NULL;
    char* result_string = NULL;
    char* subjects_string = NULL;
    char* subject = NULL;

    // Abbrechen, falls kein user angegeben wurde
    if(cmd_string == NULL) {
        printf("LIST command is invalid!\n");
        return -1;
    }

    number_mails_string = (char*) malloc((strlen("Number of mails: \n") + sizeof(int)) * sizeof(char));

    // Pfadstring für das Mailverzeichnis des Users erzeugen
    char* user = strsep(&cmd_string, "\n");
    user_dir_string = (char*) malloc((strlen(maildir) + strlen(user) + 2) * sizeof(char));
    sprintf(user_dir_string, "%s/%s", maildir, user);

    // Verzeichnis durchsuchen und Anzahl der Mails angeben
    DIR* user_dir = opendir(user_dir_string);
    if (errno == ENOENT) {
        // Das Verzeichnis gibt es noch gar nicht --> noch keine Mails für diesen User
        sprintf(number_mails_string, "Number of mails: %d\n", mails);
        send_msg(client_socket, number_mails_string);
    } else if (user_dir) {
        // Für subjects_string wird 1 Byte alloziert, damit später strlen(subjects_string) nicht zu einem 
        // Absturz führt 
        char* subjects_string = (char*) malloc(sizeof(char));
        subjects_string[0] = '\0';

        // In einer Schleife die Anzahl an Mails lesen
        struct dirent *dirp;

        while ((dirp = readdir(user_dir)) != NULL) {
            // mails inkrementieren falls es ein File ist
            if (dirp->d_type == DT_REG || dirp->d_type == DT_UNKNOWN) {
                if (readSubject(dirp, user_dir_string, &subject) != -1) {
                    mails++;
                    subjects_string = (char*) realloc(subjects_string, (strlen(subjects_string) + strlen(subject) + 1) * sizeof(char));
                    strcat(subjects_string, subject);
                }
            }
        }
        sprintf(number_mails_string, "Number of mails: %d\n", mails);

        // Antwortstring erzeugen und an den Client senden
        result_string = (char*) malloc((strlen(number_mails_string) + strlen(subjects_string) + 1) * sizeof(char));
        sprintf(result_string, "%s%s", number_mails_string, subjects_string);
        send_msg(client_socket, result_string);
    } else {
        // Fehler
        free(user_dir_string);
        return -1;
    }
    
    if (user_dir_string) {
        free(user_dir_string);
    }
    if (number_mails_string) {
        free(number_mails_string);
    }
    if (result_string) {
        free(result_string);
    }
    if (subject) {
        free(subject);
    }
    if (subjects_string) {
        free(subjects_string);
    }
    return 0;
}

// int handleRead()
// Kümmert sich um die Verarbeitung des Befehlsstrings, wenn es ein READ-Befehl ist
// gibt -1 zurück, wenn ein Fehler aufgetreten ist
//
int Server::handleRead(int client_socket) {
    printf("%s", cmd_string);
    if(cmd_string == NULL) {
        printf("READ command is invalid!\n");
        return -1;
    } 

    // Dateipfad erzeugen
    char* user = strsep(&cmd_string, "\n");
    char* file = strsep(&cmd_string, "\n");

    char* file_path = (char*) malloc((strlen(maildir) + strlen(user) + strlen(file) + 3) * sizeof(char));
    sprintf(file_path, "%s/%s/%s", maildir, user, file);

    // File öffnen
    FILE* fptr;
    if ((fptr = fopen(file_path, "r")) == NULL) {
        printf("Error opening mail\n");
        free(file_path);
        return -1;
    }

    // File auslesen
    char* mail = NULL;
    size_t mail_len;
    getdelim(&mail, &mail_len, '\0', fptr);
    mail[strlen(mail)] = '\0';

    fclose(fptr);

    // Antwortstring an Client senden
    send_msg(client_socket, mail);

    free(mail);
    free(file_path);
    return 0;
}

// int handleDel()
// Kümmert sich um die Verarbeitung des Befehlsstrings, wenn es ein DEL-Befehl ist
// gibt -1 zurück, wenn ein Fehler aufgetreten ist
//
int Server::handleDel() {
    if(cmd_string == NULL) {
        printf("DEL command is invalid!\n");
        return -1;
    } 
    
    // Dateipfad erzeugen
    char* user = strsep(&cmd_string, "\n");
    char* file = strsep(&cmd_string, "\n");

    char* file_path = (char*) malloc((strlen(maildir) + strlen(user) + strlen(file) + 3) * sizeof(char));
    sprintf(file_path, "%s/%s/%s", maildir, user, file);

    // File löschen
    if (remove(file_path) == -1) {
        free(file_path);
        return -1;
    }
    free(file_path);
    return 0;
}

// int handleLogin()
// Kümmert sich um die Verarbeitung des Befehlsstrings, wenn es ein LOGIN-Befehl ist
// gibt -1 zurück, wenn ein Fehler aufgetreten ist
//
int Server::handleLogin() {
    if(cmd_string == NULL) {
        printf("LOGIN command is invalid!\n");
        return -1;
    } 
    
    // User und Passwort vom String spalten
    char* userid = strsep(&cmd_string, "\n");
    char* passwort = strsep(&cmd_string, "\n");

    // LDAP aufrufen
    if (LDAP_search(userid) == 0) {
         LDAP_bind(userid, passwort, false);
    }

    return 0;
}

//
// --------------------------------------------------------------------------------------------------------------------------
//

//laedt credentials für LDAPSearch und speichert Sie (privat) im Server Object 
//(Passwort in Konsole nicht lesbar)
int Server::LDPA_load_Creds()
{

    std::string username;
    char* rawLdapUsername = NULL;
    do {
        // read username for LDAP search
        printf("Please insert Username for LDAP-search: ");
        getline(std::cin, username);
        rawLdapUsername = (char*) username.c_str();
        sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", rawLdapUsername);

        // read password for LDAP Search (Passwort in Konsole nicht lesbar)
        strcpy(ldapBindPassword, getpass());
    } while (LDAP_bind(ldapBindUser, ldapBindPassword, true) == -1);
    
    return 0;
}

//nimmmt die am Server hinterlegten Credentials und meldet sich damit am LDAP an
//und sucht nach der übergebenen UserID.
int Server::LDAP_search(char* userID)
{
    int rc;
    char prefix[13] = {"uid="};
    char* ldapSearchFilter= {strcat(prefix, userID)};
    LDAP* ldapHandle;

    // LDAP initialisieren
    rc = ldap_initialize(&ldapHandle, ldapUri);
    if (rc != LDAP_SUCCESS)
    {
        std::cout << "ldap_init failed!\n";
        return -1;
    } 

    // Optionen für LDAP einstellen
    rc = ldap_set_option(ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);
    if (rc != LDAP_OPT_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle, nullptr,nullptr);
        std::cout << "\nset option failed!\n";
        return -1;
    }

    // TLS-Verbindung starten
    rc = ldap_start_tls_s(
        ldapHandle,
        nullptr,
        nullptr
    );

    if (rc != LDAP_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle, nullptr,nullptr);
        std::cout << "\nstart tls failed!\n";
        return -1;
    }

    // Authentifizierungsprotokoll SASL nutzen
    BerValue bindCredentials;
    bindCredentials.bv_val = ldapBindPassword;
    bindCredentials.bv_len = strlen(ldapBindPassword);
    BerValue *servercredp; // server's credentials
    rc = ldap_sasl_bind_s(

        ldapHandle,
        ldapBindUser,
        LDAP_SASL_SIMPLE,
        &bindCredentials,
        NULL,
        NULL,
        &servercredp);

    if (rc !=  LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ldapHandle, nullptr, nullptr);
        std::cout << "\nsasl_bind failed!\n";
        return -1;
    }

    // perform ldap search
    LDAPMessage *searchResult;
    rc = ldap_search_ext_s(
        ldapHandle,
        ldapSearchBaseDomainComponent,
        ldapSearchScope,
        ldapSearchFilter,
        (char **)ldapSearchResultAttributes,
        0,
        NULL,
        NULL,
        NULL,
        500,
        &searchResult);

    if (rc != LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        std::cout << "\nldap search failed!\n";
        return -1; 
    }

    // Einträge anzeigen
    printf("Total results: %d\n", ldap_count_entries(ldapHandle, searchResult));

    if (ldap_count_entries(ldapHandle, searchResult) != 1) {
        printf("No user was found!\n");
        return -1;
    } 

    // get result of search
    LDAPMessage *searchResultEntry;
    for (searchResultEntry = ldap_first_entry(ldapHandle, searchResult);
        searchResultEntry != NULL;
        searchResultEntry = ldap_next_entry(ldapHandle, searchResultEntry))
    {
        // Base Information of the search result entry
        printf("DN: %s\n", ldap_get_dn(ldapHandle, searchResultEntry));

        BerElement *ber;
        char *searchResultEntryAttribute;
        for (searchResultEntryAttribute = ldap_first_attribute(ldapHandle, searchResultEntry, &ber);
            searchResultEntryAttribute != NULL;
            searchResultEntryAttribute = ldap_next_attribute(ldapHandle, searchResultEntry, ber))
        {
            BerValue **vals;
            if ((vals = ldap_get_values_len(ldapHandle, searchResultEntry, searchResultEntryAttribute)) != NULL)
            {
            for (int i = 0; i < ldap_count_values_len(vals); i++)
            {
                printf("\t%s: %s\n", searchResultEntryAttribute, vals[i]->bv_val);
            }
            ldap_value_free_len(vals);
            }

            // free memory
            ldap_memfree(searchResultEntryAttribute);
        }
        // free memory
        if (ber != NULL)
        {
            ber_free(ber, 0);
        }

        printf("\n");
    }

    // free memory
    ldap_msgfree(searchResult);
    ldap_unbind_ext_s(ldapHandle, NULL, NULL);

    return 0;
}

//nimmmt die an die Funktion übergebenen Credentials und macht 
//damit am LDAP ein bind - gibt jeweils 0 or -1 retour
int Server::LDAP_bind(char* userID, char* passwort, bool isServer)
{
    int rc;
    char* ldapUser;
    if (!isServer) {
        char prefix[47] = {"uid="};
        char* subfix = ",ou=people,dc=technikum-wien,dc=at";
        char* temp = strcat(prefix, userID); 
        ldapUser = strcat(temp, subfix);
    } else {
        ldapUser = userID;
    }
    LDAP* ldapHandle;
    
    rc = ldap_initialize(&ldapHandle, ldapUri);
    if (rc != LDAP_SUCCESS)
    {
        std::cout << "ldap_init failed!\n";
        return -1;
    }

    rc = ldap_set_option(
        ldapHandle,
        LDAP_OPT_PROTOCOL_VERSION,
        &ldapVersion);
    if (rc != LDAP_OPT_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle, nullptr, nullptr);
        std::cout << "\nset option failed!\n";
        return -1;
    }

    rc = ldap_start_tls_s(
        ldapHandle,
        nullptr,
        nullptr
    );

    if (rc != LDAP_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle, nullptr,nullptr);
        std::cout << "\nstart tls failed!\n";
        return -1;

    }

    BerValue bindCredentials;
    bindCredentials.bv_val = passwort;
    bindCredentials.bv_len = strlen(passwort);
    BerValue *servercredp; // server's credentials
    rc = ldap_sasl_bind_s(
        ldapHandle,
        ldapUser,
        LDAP_SASL_SIMPLE,
        &bindCredentials,
        NULL,
        NULL,
        &servercredp);

    if (rc !=  LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ldapHandle, nullptr, nullptr);
        std::cout << rc;
        return -1;
    }

    ldap_unbind_ext_s(ldapHandle, nullptr, nullptr);
    return 0;
}