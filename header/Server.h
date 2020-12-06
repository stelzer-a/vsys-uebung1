#ifndef TWMAILER_SERVER
#define TWMAILER_SERVER

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
#include <pthread.h>


#include "Mail.h"
#include "../util/util.h"

#define BUF 128

class Server {
    private:
        // Membervariablen
        char* maildir;
        char* recv_buffer;
        char* cmd_string;
        int buffer_size;

        // Membervariablen -- Sockets
        int listen_socket, client_socket;
        struct sockaddr_in srv_addr_struct, clt_addr_struct;
        socklen_t sock_addr_len;

        //LDAP Variablen
        const char* ldapUri = "ldap://ldap.technikum-wien.at:389";
        const int ldapVersion = LDAP_VERSION3;
        char ldapBindUser[BUF*2];
        char ldapBindPassword[BUF*2];
        const char* ldapSearchBaseDomainComponent = "dc=technikum-wien,dc=at";
        ber_int_t ldapSearchScope = LDAP_SCOPE_SUBTREE;
        const char* ldapSearchResultAttributes[5] = {"uid", "cn", nullptr};

        int acceptClient();
        int parseCmd();
        int readSubject(struct dirent *dirp, char* path, char** subject);

        // Handle Befehl Funktionen
        int handleSend();
        int handleList();
        int handleRead();
        int handleDel();
        int handleLogin();
    public:
        // Konstruktor und Destruktor
        Server();
        ~Server();

        int init(int server_port, char* maildir);
        int start();
        void stop();

        //LDAP Funktionen
        int LDPA_load_Creds();
        int LDAP_search(char* userID);
        int LDAP_bind(char* userID, char* passwort, bool isServer);

        void send_msg(char* msg);
        void recv_cmd();
};

#endif