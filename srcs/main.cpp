

#include "../includes/server.hpp"

int main()
{
    server a;
    if (a.creat_sokect() == EXIT_FAILURE)
        return 1;
    a.listen_and_accept();
}