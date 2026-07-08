#ifndef CHANNEL_HPP
#define	CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include "client.hpp"
#include <ctime>


class Channel
{
	private:
		std::string name;
		std::string topic;
		time_t creationTime;
		std::string channelKey;
		bool inviteOnly;
		bool topicRestricted;
		bool Locked;
		bool Limited;
		size_t maxLimit;
		std::set<int> members;
		std::set<int> operators;
		std::set<int> invited;
	public:
		Channel(std::string n);
		Channel(){};
		~Channel();
		const std::set<int> & getMembers() const;
		void addMember(int clientFd);
		void addOperator(int clientFd);
		void removeMember(int clientFd);
		void removeInvite(int clientFd);
		void removeOperator(int clientFd);
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
		const time_t &getCreationTime() const;
		void setTopic(std::string topic);
		void setTopicRestriction(bool state);
		void setChannelKey(std::string key);
		void setLimitState(bool state);
		void setMaxLimit(size_t limit);
		void setInviteOnly(bool state);
		void setLocked(bool state);
		void clearTopic();

};
bool validateChannelName(std::string name);
std::vector<std::string> splitArgument(std::string & arguments);
void printChannelModes(client * currentClient, std::string & command, std::map<std::string, Channel>::iterator &it);

#endif
