
#include "../includes/server.hpp"

server::server():  socket_fd(-1), myPort("8080"),reuse_flag(1), client_fd(-1)
{
}

int server::creat_sokect()
{
    struct addrinfo hints, *serverinfo;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    
    if (getaddrinfo(NULL, myPort, &hints, &serverinfo) != 0)
    {
        std::cerr << "ERROR: getaddrinfo failed.\n";
        return EXIT_FAILURE;
    }
    this->socket_fd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (this->socket_fd < 0)
    {
        std::cout << "ERROR: creation of socket failed.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &this->reuse_flag, sizeof(this->reuse_flag)) == -1)
    {
        std::cout << "ERROR: failed to reuse socket.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }
    
    if (bind(this->socket_fd, serverinfo->ai_addr, serverinfo->ai_addrlen) ==  -1)
    {
        std::cout << "ERROR: bind the socket failed. " << "\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }
    freeaddrinfo(serverinfo);
    return EXIT_SUCCESS;
}

int server::listen_and_accept()
{
    if (listen(this->socket_fd, SOMAXCONN) == -1)
    {
        std::cout << "ERROR: Failed to start listening for incoming connections.\n";
        return EXIT_FAILURE;
    }
    // may be i need handel signals here

    std::cout << "server: waiting for connections...\n";

    struct sockaddr_storage client_sockaddr;
    socklen_t client_sockaddr_size;
    while (1)
    {// when the while stand and wait here or below
        client_sockaddr_size = sizeof(client_sockaddr);
        this->client_fd = accept(this->socket_fd, (struct sockaddr*)&client_sockaddr, &client_sockaddr_size);
        if (this->client_fd < 0)
        {
            std::cout << "ERROR: accept clinet socket failed.\n";
            return EXIT_FAILURE;
        }

    }
    

}

server::~server(){};
