#include <sys/socket.h> // for socket()
#include <netinet/in.h> // for struct sockaddr_in 
#include <netdb.h> // for getprotobyname
#include <iostream>

class server
{
    private:
        int port, socket_fd;
    public:
        server();
        ~server();
        int creat_sokect();

};