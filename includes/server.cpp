
#include "server.hpp"

int server::creat_sokect()
{
    if ((server::socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "ERROR: creation of socket fialed";
        return EXIT_FAILURE;
    }
    
    bind(server::socket_fd, (struct sockaddr *)&add, sizeof(addr))
}