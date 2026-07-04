
#include "../includes/client.hpp"

client::client() : authentication(false), passOk(false), hasNick(false), hasUser(false)
{};

void client::setFD(int FD)
{
    this->fd = FD;
}
void client::setIpAdd(std::string CIpAdd)
{
    this->IpAdd = CIpAdd;
}

void client::setNickName(std::string nickName)
{
    this->nickName = nickName;
}

void client::setUserName(std::string userName)
{
    this->userName = userName;
}

void client::setRealName(std::string realName)
{
    this->realName = realName;
}

int client::getFD()
{
    return this->fd;
};

std::string client::getIpAdd()
{
    return this->IpAdd;
};
const std::string &client::getUserName() const
{
	return this->userName;
}

const std::string &client::getNickName() const
{
	return this->nickName;
}


bool client::isAuthenticat()
{
    return authentication;
}

void client::setAsAuthenticated()
{
    authentication = true;
}

void client::setPassStatusFalse()
{
    this->passOk = false;
};

void client::clientSetBuff(std::vector<std::string> buff)
{
    this->buffer = buff;
}

std::vector<std::string> client::clientGetBuff()
{
    return (this->buffer);
}

void client::setAuthenRequirment(int id)
{
    if (id == 1)
        this->passOk = true;
    else if (id == 2)
        this->hasNick = true;
    else if (id==3)
        this->hasUser = true;
}

bool client::checkAuthenRequirment()
{
    if (this->hasNick == true && hasUser==true && passOk == true)
        return true;
    return false;
};

client::~client(){};
