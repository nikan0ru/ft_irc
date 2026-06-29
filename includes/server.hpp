#include <sys/socket.h> // for socket()
#include <netinet/in.h> // for struct sockaddr_in 
#include <netdb.h> // for getprotobyname
#include <unistd.h> // Required for gethostname
#include <cstring>
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "client.hpp"
#include <arpa/inet.h>

class server
{
    private:
        int socket_fd;
        int reuse_flag;
        const std::string& servport;
        const std::string& servpass;
        int client_fd;
        std::vector<struct pollfd> pollfds;
        std::vector<client> clients;
    public:
        server(const std::string& portnum, const std::string& authpass);
        ~server();
        int creat_sokect();
        client *getClient(int fd);
        int listen_and_monitorfdstatus();
        int procces_connections();
        int acceptNewClient();
        int handelNewData(int cliFd);
        void parse_and_exe(client *curClient, std::vector<std::string> splited_cmd);
        std::vector<std::string> split_recved_buffer(std::string buff);
        std::vector<std::string> splited_cmd(std::string& cmd);
        void closeAllFds();
        void removeFd(int fd);
        void removeClient(int fd);


};