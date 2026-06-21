#include <sys/socket.h> // for socket()
#include <netinet/in.h> // for struct sockaddr_in 
#include <netdb.h> // for getprotobyname
#include <unistd.h> // Required for gethostname
#include <cstring>
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include "client.hpp"
#include <arpa/inet.h>

class server
{
    private:
        int socket_fd;
        int reuse_flag;
        const std::string& servport;
        // const std::string& servpass;
        int client_fd;
        std::vector<struct pollfd> pollfds;
        std::vector<client> clients;
    public:
        server(const std::string& portnum, const std::string& authpass);
        ~server();
        int creat_sokect();
        int listen_and_monitorfdstatus();
        int procces_connections();
        int acceptNewClient();
        int handelNewData();
        void closeAllFds();
};