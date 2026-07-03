#include "../includes/channel.hpp"
#include <iostream>

Channel::Channel(std::string n) : name(n), topic(""), InviteOnly(false), TopicRestricted(false),Locked(false), Limited(false), MaxLimit(0)
{

}

const std::set<int> & Channel::getMembers() const
{
	for (size_t i = 0 ; i < this->members.size(); i++)
		std::cout << this->members[i] << "sasa"<< std::endl;
	// return this->members;
}
void Channel::addMember(int clientFd)
{
	if (this->members.size() == 0)
		this->operators.insert(clientFd);
	this->members.insert(clientFd);
}

void Channel::removeMember(int clientFd)
{
	this->members.erase(clientFd);
	if(this->isOperator(clientFd))
		this->operators.erase(clientFd);
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

bool Channel::isInvited(int clientFd)
{
	std::set<int>::iterator it;

	it = this->invited.find(clientFd);
	if(it != this->invited.end())
		return true;
	return false;
}

size_t Channel::getMaxLimit() const
{
	return this->MaxLimit;
}


const std::string& Channel::getChannelName() const
{
	return this->name;
}
const std::string &Channel::getTopic() const
{
	return this->topic;
}

const std::string &Channel::getChannelKey() const
{
	return this->channelKey;
}

bool Channel::isLocked() const
{
	return this->Locked;
}
bool Channel::isInviteOnly() const
{
	return this->InviteOnly;
}
bool Channel::isTopicRestricted() const
{
	return this->TopicRestricted;
}
bool Channel::isLimited() const
{
	return this->Limited;
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

std::vector<std::string> splitArgument(std::string & arguments)
{
	std::vector<std::string> splitArgs;
	size_t start;
	size_t comma;

	start = 0;
	while (true)
	{
		comma = arguments.find(',', start);
		if(comma == std::string::npos)
		{
			splitArgs.push_back(arguments.substr(start));
			break;
		}
		splitArgs.push_back(arguments.substr(start, comma - start));
		start = comma + 1;
	}
	return splitArgs;
}
