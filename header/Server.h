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
    public:
        // Konstruktor und Destruktor
        Server();
        ~Server();

        int init(int server_port, char* maildir);
        int start();
        void stop();

        int send_msg(char* msg);
        void recv_cmd();
};