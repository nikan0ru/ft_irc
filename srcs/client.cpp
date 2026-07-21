
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

const std::string client::getClientName() const
{
	std::string ClientName;

	ClientName = this->getNickName() + "!" + this->getUserName() + "@" + this->getIpAdd();
	return ClientName;
}

int client::getFD()
{
    return this->fd;
};

const std::string client::getIpAdd() const
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

const std::string &client::getRealName() const
{
	return this->realName;
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

bool client::checkAuthenRequirment(int i)
{
    if (i == 1 && passOk == true)
        return true;
    if (i == 2 && passOk == true && hasNick == true)
        return true;
    if (i == 3 && hasNick == true && hasUser==true && passOk == true)
        return true;
    return false;
};

client::~client(){};
