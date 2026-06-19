#include <sys/socket.h> // for socket()
#include <netinet/in.h> // for struct sockaddr_in 
#include <netdb.h> // for getprotobyname
#include <unistd.h> // Required for gethostname
#include <cstring>
#include <iostream>

class server
{
    private:
        int socket_fd;
        char myPort[5];
        int reuse_flag;
    public:
        server();
        ~server();
        int creat_sokect();

};