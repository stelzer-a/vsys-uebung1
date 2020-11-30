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

#include "Mail.h"
#include "../util/util.h"

#define BUF 5
#define HEADER_BYTES 4
#define MAX_USERID_SIZE 8
#define MAX_SUBJECT_SIZE 80
#define OK_STRING "OK\n"
#define ERR_STRING "ERR\n"

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

        int acceptClient();
        int parseCmd();

        // Handle Befehl Funktionen
        int handleSend();
        int handleList();
        int handleRead();
        int handleDel();
    public:
        // Konstruktor und Destruktor
        Server();
        ~Server();

        int init(int server_port, char* maildir);
        int start();
        void stop();

        void send_msg(char* msg);
        void recv_cmd();
};

#endif