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
	int memberCount;
	std::vector<client *> members;
	std::vector<client *> operators;

public:
	Channel(std::string n);
	~Channel();
	std::vector<client *> & getMembers();
	void addMember(client * clnt);
	bool isMember(client &clnt);
	bool isOperator(client &clnt);
	const std::string &getChannelName() const;
	const std::string &getTopic() const;
	void setTopic(std::string topic);
	void clearTopic();

};
bool validateChannelName(std::string name);
#endif
