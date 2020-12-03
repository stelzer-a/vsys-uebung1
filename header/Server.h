#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <ldap.h>
#include <iostream>
#include "../header/mypw.h"
#include <uuid/uuid.h>

#define BUF 1024

class Server {
    private:
        // Membervariablen
        char* maildir;
        char* recv_buffer;

        // Membervariablen -- Sockets
        int listen_socket, client_socket;
        struct sockaddr_in srv_addr_struct, clt_addr_struct;
        socklen_t sock_addr_len;

        int acceptClient();

        //LDAP Variablen
        const char* ldapUri = "ldap://ldap.technikum-wien.at:389";
        const int ldapVersion = LDAP_VERSION3;
        char ldapBindUser[256];
        char ldapBindPassword[256];
        const char* ldapSearchBaseDomainComponent = "dc=technikum-wien,dc=at";
        ber_int_t ldapSearchScope = LDAP_SCOPE_SUBTREE;
        const char* ldapSearchResultAttributes[5] = {"uid", "cn", nullptr};

    public:
        // Konstruktor und Destruktor
        Server();
        ~Server();

        int init(int server_port, char* maildir);
        int start();

        //LDAP Funktionen
        int LDPA_load_Creds();
        void LDAP_search(char* userID);
        bool LDAP_bind(char* userID, char* passwort);

        //UUID
        std::string generate_UUID();

        void stop();

        int send_msg(char* msg);
        void recv_cmd();
};