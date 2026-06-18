
#include "server.hpp"

int server::creat_sokect()
{
    struct sockaddr_in addr; 

    if ((server::socket_fd = socket(PF_INET, SOCK_STREAM, getprotobyname("TCP"))) < 0)
    {
        std::cout << "ERROR: creation of socket fialed";
        return EXIT_FAILURE;
    }
    
    // addr.sin

    bind(server::socket_fd, (struct sockaddr *)&add, sizeof(addr))
}