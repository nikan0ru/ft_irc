
#include "../includes/server.hpp"

server::server():  socket_fd(-1), myPort("9000"),reuse_flag(1)
{
    int status = creat_sokect();
    (void)(status);
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
        std::cerr << "ERROR: getaddrinfo failed\n" << std::endl;
        return EXIT_FAILURE;
    }
    this->socket_fd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (this->socket_fd < 0)
    {
        std::cout << "ERROR: creation of socket failed\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &this->reuse_flag, sizeof(this->reuse_flag)) == -1)
    {
        std::cout << "ERROR: failed to reuse socket\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }
    
    if ( bind(this->socket_fd, serverinfo->ai_addr, serverinfo->ai_addrlen) ==  -1)
    {
        std::cout << "ERROR: bind the socket failed: "<< serverinfo->ai_addr<<  strerror(errno) << "\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    freeaddrinfo(serverinfo);

    return EXIT_SUCCESS;
}

server::~server(){};
