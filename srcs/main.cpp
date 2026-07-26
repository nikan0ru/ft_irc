

#include "../includes/server.hpp"

int main(int ac, char *av[])
{
    if (ac != 3 || !av[1][0] || !av[2][0] )
        return EXIT_FAILURE;

    std::string password = av[2];
    if (password.length() > 505)
    {
        std::cout << "ERROR: Password is too long! Max length is 505 characters\n";
        return EXIT_FAILURE;
    }

    server a(av[1], av[2]);
    if (a.creat_sokect())
        return EXIT_FAILURE;
    a.listen_and_monitorfdstatus();
}