#include "../includes/channel.hpp"

Channel::Channel(std::string n) : name(n), memberCount(0)
{
}

std::vector<client *> & Channel::getMembers()
{
	return this->members;
}
void Channel::addMember(client *clnt)
{
	if (this->memberCount == 0)
		this->operators.push_back(clnt);
	this->members.push_back(clnt);
	memberCount++;
}
bool Channel::isOperator(client &clnt)
{
	for (size_t i = 0; i < this->operators.size(); i++)
	{
		if (this->operators.at(i) == &clnt)
			return true;
	}
	return false;
}

bool Channel::isMember(client &clnt)
{
	for (size_t i = 0; i < this->operators.size(); i++)
	{
		if (this->operators.at(i) == &clnt)
			return true;
	}
	return false;
}

const std::string& Channel::getChannelName() const
{
	return this->name;
}
const std::string &Channel::getTopic() const
{
	return this->topic;
}

void Channel::setTopic(std::string newTopic)
{
	this->topic = newTopic;
}

void Channel::clearTopic()
{
	this->topic.clear();
}

Channel::~Channel()
{

}

bool validateChannelName(std::string name)
{
	if(name[0] != '#' && name[0] != '&')
		return 0;
	for (size_t i = 0; i < name.size(); i++)
	{
		if(name[i] == ' ' || name[i] == ',' || name[i] == '\a')
			return 0;
	}
	return 1;
}
