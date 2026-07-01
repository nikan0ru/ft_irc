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
	bool InviteOnly;
	bool TopicRestricted;
	bool Locked;
	bool Limited;
	int MaxLimit;
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
	bool IsLocked() const;
	bool IsInviteOnly() const;
	bool IsInvited() ;
	bool IsTopicRestricted() const;
	bool isLimited() const;
	const int getMaxLimit() const;
	const std::string &getChannelName() const;
	const std::string &getTopic() const;
	void setTopic(std::string topic);
	void clearTopic();

};
bool validateChannelName(std::string name);
#endif
