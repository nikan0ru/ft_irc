#include "../includes/channel.hpp"

Channel::Channel(std::string n) : name(n)
{
}

const std::set<int> & Channel::getMembers() const
{
	return this->members;
}
void Channel::addMember(int clientFd)
{
	if (this->members.size() == 0)
		this->operators.insert(clientFd);
	this->members.insert(clientFd);
}
bool Channel::isOperator(int clientFd)
{
	std::set<int>::iterator it;

	it = this->operators.find(clientFd);
	if(it != this->operators.end())
		return true;
	return false;
}

bool Channel::isMember(int clientFd)
{
	std::set<int>::iterator it;

	it = this->members.find(clientFd);
	if(it != this->members.end())
		return true;
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
	if(name.empty())
		return false;
	if(name[0] != '#' && name[0] != '&')
		return false;
	for (size_t i = 0; i < name.size(); i++)
	{
		if(name[i] == ' ' || name[i] == ',' || name[i] == '\a')
			return false;
	}
	return true;
}
