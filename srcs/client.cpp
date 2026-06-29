
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
std::string client::getIpAdd()
{
    return this->IpAdd;
};
const std::string &client::getUserName()
{
	return this->userName;
}

void client::clientSetBuff(std::vector<std::string> buff)
{
    this->buffer = buff;
}

std::vector<std::string> client::clientGetBuff()
{
    return (this->buffer);
}

client::~client(){};
