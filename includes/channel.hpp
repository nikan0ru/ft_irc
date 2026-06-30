#ifndef CHANNEL_HPP
#define	CHANNEL_HPP

#include <string>
#include <vector>
#include "client.hpp"

class Channel
{
private:
	std::string name;
	std::string topic;
	std::set<int> members;
	std::set<int> operators;

public:
	Channel(std::string n);
	~Channel();
	const std::set<int> & getMembers() const;
	void addMember(int clientFd);
	bool isMember(int clientFd);
	bool isOperator(int clientFd);
	const std::string &getChannelName() const;
	const std::string &getTopic() const;
	void setTopic(std::string topic);
	void clearTopic();

};
bool validateChannelName(std::string name);
#endif
