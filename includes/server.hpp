#include <sys/socket.h>
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