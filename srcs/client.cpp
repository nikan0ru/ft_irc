


#include "../includes/client.hpp"

client::client(){};

void client::setFD(int FD)
{
    this->fd = FD; 
}
void client::setIpAdd(std::string CIpAdd)
{
    this->IpAdd = CIpAdd;
}
int client::getFD()
{
    return this->fd;
};

client::~client(){};