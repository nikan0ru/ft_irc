#include <iostream>
#include <vector>
#include <map>
#include <set>

class Client
{
private:
	int fd;
	std::string userName;
	std::string nickName;
	bool isAuthenticated;
	bool isRegistered;
	std::set<std::string> channelsJoined;
public:
	Client() : fd(5), userName("pop"), nickName("nicky"), isAuthenticated(true)
	{
	}
	~Client()
	{
	}
	const std::string &getUserName()
	{
		return this->userName;
	}
};

class Channel
{
private:
	std::string name;
	std::vector<Client *> members;
	std::vector<Client *> operators;

public:
	bool isOperator(Client &client)
	{
		for (size_t i = 0; i < this->members.size(); i++)
		{
			if (this->members.at(i) == &client)
				return true;
		}
		return false;
	}
};

class Server
{
private:
	std::map<int, Client *> Clients;
	std::map<std::string, Channel *> Channels;
	std::string password;
	int port;

public:
	Channel *getChannel(const std::string &name);
};

int main()
{
	Client pop;
	Channel general;
	std::cout << pop.getUserName();

	return 0;
}
