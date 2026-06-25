


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

void client::clientSetBuff(std::vector<std::string> buff)
{
    this->buffer = buff;
}

client::~client(){};