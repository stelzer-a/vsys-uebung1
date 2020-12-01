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

//-------- LDAP Implementrierung --------- 

//laedt credentials für LDAPSearch und speichert Sie (privat) im Server Object 
//(Passwort in Konsole nicht lesbar)
int Server::LDPA_load_Creds()
{
// read username for LDAP search
    std::string username;
    printf("Please insert Username for LDAP-search: ");
    getline(std::cin, username);
    const char* rawLdapUsername = username.c_str();
    sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", rawLdapUsername);
   
 // read password for LDAP Search (Passwort in Konsole nicht lesbar)
    strcpy(ldapBindPassword, getpass());
    printf("pw taken over from commandline\n");

    return 0;
}

//nimmmt die am Server hinterlegten Credentials und meldet sich damit am LDAP an
//und sucht nach der übergebenen UserID.
void Server::LDAP_search(std::string search_uid)

{
    int rc;
    const char* ldapSearchFilter = search_uid.c_str();
    LDAP* ldapHandle;
    rc = ldap_initialize(&ldapHandle, ldapUri);
    if (rc != LDAP_SUCCESS)
    {
        std::cout << "ldap_init failed!\n";
    } 
    std::cout << "connected to LDAP\n" << ldapUri;

    rc = ldap_set_option(
        ldapHandle,
        LDAP_OPT_PROTOCOL_VERSION,
        &ldapVersion);
    if (rc != LDAP_OPT_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle, nullptr,nullptr);
        std::cout << "\nset option failed!\n";
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
    }

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
    }

    printf("Total results: %d\n", ldap_count_entries(ldapHandle, searchResult));

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
}

//nimmmt die an die Funktion übergebenen Credentials und macht 
//damit am LDAP ein bind - gibt jeweils true or false retour
bool Server::LDAP_bind(char* userID_, char* passwort)

{
    int rc;
    LDAP* ldapHandle_;
    char* ldapBindUser_ = userID_; 
    rc = ldap_initialize(&ldapHandle_, ldapUri);
    if (rc != LDAP_SUCCESS)
    {
        std::cout << "ldap_init failed!\n";
    } 
    std::cout << "connected to LDAP\n" << ldapUri;

    rc = ldap_set_option(
        ldapHandle_,
        LDAP_OPT_PROTOCOL_VERSION,
        &ldapVersion);
    if (rc != LDAP_OPT_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle_, nullptr, nullptr);
        std::cout << "\nset option failed!\n";
    }

    rc = ldap_start_tls_s(
        ldapHandle_,
        nullptr,
        nullptr
    );

    if (rc != LDAP_SUCCESS) 
    {
        ldap_unbind_ext_s(ldapHandle_, nullptr,nullptr);
        std::cout << "\nstart tls failed!\n";
    }

    BerValue bindCredentials;
    bindCredentials.bv_val = passwort;
    bindCredentials.bv_len = strlen(passwort);
    BerValue *servercredp; // server's credentials
    rc = ldap_sasl_bind_s(
        ldapHandle_,
        ldapBindUser_,
        LDAP_SASL_SIMPLE,
        &bindCredentials,
        NULL,
        NULL,
        &servercredp);

    if (rc !=  LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ldapHandle_, nullptr, nullptr);
        std::cout << rc;
        return false;
    }

    ldap_unbind_ext_s(ldapHandle_, nullptr, nullptr);
    return true;
}

//erzeut eine UUID und gibt diese als Strin retour.
std::string Server::generate_UUID()
{
    uuid_t binuuid; //binaryUUID
    char uuid[37]; //laenge einer UUID (36-byte string + tailing \0)

    uuid_generate_random(binuuid); //generate binary UUID
    uuid_unparse_lower(binuuid,uuid); //converts binary UUID to string
    std::string uuidStr = uuid;

    return uuidStr;
}