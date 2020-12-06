#ifndef TWMAILER_CLIENT
#define TWMAILER_CLIENT

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../util/util.h"

#define BUF 1024
#define ERR_USER "xxxxxxxxxxxx"

class Client {
    private:
        // Private Membervariablen
        int client_socket;
        struct sockaddr_in srv_addr_struct;
        char* recv_buffer;

        int readCmd(char* cmd);

        char* userid;
        char pw[BUF/4];

    public:
        // Konstruktor und Destruktor
        Client();
        ~Client();

        // Methode init() benötigt Addresse und Port des Servers
        int init(char* srv_addr, int srv_port);

        int connectToServer();
        int disconnect();

        void send_cmd();
        int receive(char* cmd);

};

#endif