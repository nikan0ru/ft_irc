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
	std::string channelKey;
	bool InviteOnly;
	bool TopicRestricted;
	bool Locked;
	bool Limited;
	size_t MaxLimit;
	std::set<int> members;
	std::set<int> operators;
	std::set<int> invited;
public:
	Channel(std::string n);
	~Channel();
	const std::set<int> & getMembers() const;
	void addMember(int clientFd);
	void removeMember(int clientFd);
	bool isMember(int clientFd);
	bool isOperator(int clientFd);
	bool isInvited(int clientFd) ;
	bool isLocked() const;
	bool isInviteOnly() const;
	bool isTopicRestricted() const;
	bool isLimited() const;
	size_t getMaxLimit() const;
	const std::string &getChannelKey() const;
	const std::string &getChannelName() const;
	const std::string &getTopic() const;
	void setTopic(std::string topic);
	void clearTopic();

};
bool validateChannelName(std::string name);
std::vector<std::string> splitArgument(std::string & arguments);
#endif
