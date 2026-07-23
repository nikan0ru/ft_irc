#include "../includes/server.hpp"


void server::handleKick(client* currentClient, std::vector<std::string>& cmd)
{
	std::vector<std::string> nickList;
	std::vector<std::string> channelList;
	std::string kickReason;
	std::string message;
    std::map<std::string, Channel>::iterator it;
	size_t channelListsize;

    if (!currentClient->isAuthenticat())
	{
		sendErrorMessage(currentClient, "KICK", " :You have not registered", "451");
		return;
	}
    if (cmd.size() < 3)
	{
		sendErrorMessage(currentClient, "KICK", " :Not enough parameters", "461");
        return;
	}
	channelList = splitArgument(cmd[1]);
    nickList = splitArgument(cmd[2]);

	channelListsize = channelList.size();
	if (nickList.size() < channelListsize)
		channelListsize = nickList.size();

	for (size_t i = 0; i < channelListsize; i++)
	{
		it = this->Channels.find(normalize(channelList[i]));
		if(it == this->Channels.end())
		{
			sendErrorMessage(currentClient, channelList[i], " :No such channel", "403");
			continue;
		}
		if (!it->second.isMember(currentClient->getFD()))
		{
			sendErrorMessage(currentClient, channelList[i], " :You're not on that channel", "442");
			continue;
		}
		if (!it->second.isOperator(currentClient->getFD()))
		{
			sendErrorMessage(currentClient, channelList[i], " :You're not channel operator", "482");
			continue;
		}
		for (size_t j = 0; j < this->clients.size(); j++)
		{
			if (normalize(this->clients[j].getNickName()) == normalize(nickList[i]))
			{
				if (!it->second.isMember(this->clients[j].getFD()))
				{
					sendErrorMessage(currentClient, nickList[i] + " " + channelList[i], " :They aren't on that channel", "441");
					break;
				}

				kickReason = nickList[i];
				if (cmd.size() > 3)
					kickReason = cmd[3];

				message = ":" + currentClient->getNickName() + "!" + currentClient->getUserName() + "@" + currentClient->getIpAdd()
					+ " KICK " + channelList[i] + " " + nickList[i] + " :" + kickReason + "\r\n";
				for (size_t k = 0; k < this->clients.size(); k++)
				{
					if (it->second.isMember(this->clients[k].getFD()))
					{

						send(this->clients[k].getFD(), message.c_str(), message.length(), 0);
					}
				}

				if (it->second.isOperator(this->clients[j].getFD()))
					it->second.removeOperator(this->clients[j].getFD());
				it->second.removeMember(this->clients[j].getFD());
				if(it->second.getMembers().size() == 0)
					this->Channels.erase(it);
				break;
			}
		}
	}
}



void server::handleInvite(client* curr_client, std::vector<std::string>& cmd)
{
    int cmdsize = cmd.size();
    if (!curr_client->isAuthenticat())
        return (sendErrorMessage(curr_client, "INVITE", " :You have not registered", "451"), void());
    if (cmdsize < 3)
        return (sendErrorMessage(curr_client, "INVITE", " :Not enough parameters", "461"), void());

    std::string targetNick =  cmd[1],targetChannel = cmd[2];
    std::map<std::string, Channel>::iterator it;
	client* targetClient = NULL;

	it = this->Channels.find(normalize(targetChannel));
    for (size_t i = 0; i < this->clients.size(); i++)
		if (normalize(clients[i].getNickName()) == normalize(targetNick))
			targetClient = &clients[i];

	if (targetClient == NULL)
		return (sendErrorMessage(curr_client, "INVITE", " : No such nick", "401"), void());

	if(it != this->Channels.end())
	{
		if(!it->second.isMember(curr_client->getFD()))
			return (sendErrorMessage(curr_client,cmd[0], " :You're not on that channel", "442"), void());
		if(it->second.isInviteOnly())
        	if (!it->second.isOperator(curr_client->getFD()))
            	return (sendErrorMessage(curr_client,cmd[0], " :You're not channel operator", "482"), void());
		if(it->second.isMember(targetClient->getFD()))
		{
			std::string textMsg = ":ircserv 443 " + curr_client->getNickName() + " " +
					targetNick + " " + targetChannel + " :is already on channel\r\n";
			return (send(curr_client->getFD(), textMsg.c_str(), textMsg.size(), 0), void());
		}
		it->second.addInvited(targetClient->getFD());
	}

    std::string textMsg = ":ircserv 341 " + curr_client->getNickName() + \
        " " + targetNick + " " + targetChannel + "\r\n";
    send(curr_client->getFD(), textMsg.c_str(), textMsg.size(), 0);
    textMsg = ":" + curr_client->getNickName() + "!" + curr_client->getUserName() + "@" + curr_client->getIpAdd()
                + " " + "INVITE" + " " +targetNick + " " + targetChannel + "\r\n";
    send(targetClient->getFD(), textMsg.c_str(), textMsg.size(), 0);
}


void server::handlePrivmsg(client* curr_client, std::vector<std::string>& cmd)
{
    int cmdsize = cmd.size();
    if (!curr_client->isAuthenticat())
        return (sendErrorMessage(curr_client, "PRIVMSG", " :You have not registered", "451"), void());
    if (cmdsize < 2)
        return (sendErrorMessage(curr_client, "PRIVMSG", " :No recipient given (PRIVMSG)", "411"), void());
    if (cmdsize < 3)
        return (sendErrorMessage(curr_client, "PRIVMSG", " :No text to send", "412"), void());

    std::vector<std::string> targets = splitArgument(cmd[1]);
    std::map<std::string, Channel>::iterator it;
    size_t targetSize = targets.size();
    std::string textMsg;

    for (size_t i = 0; i < targetSize; i++)
    {
        std::string target = targets[i];
        if (target[0] == '&' || target[0] == '#')
        {
            if ((it = Channels.find(normalize(target))) == Channels.end())
            {
                sendErrorMessage(curr_client, target, " :No such channel", "403");
                continue;
            }
            if (!it->second.isMember(curr_client->getFD()))
            {
                sendErrorMessage(curr_client, target, " :Cannot send to channel", "404");
                continue;
            }

            textMsg = ":" + curr_client->getClientName() + " PRIVMSG " + it->second.getChannelName() + " :" + cmd[2] +"\r\n";
			for (std::set<int>::iterator sit = it->second.getMembers().begin(); sit != it->second.getMembers().end(); sit++)
			{
				if(*sit != curr_client->getFD())
					send(*sit, textMsg.c_str(), textMsg.size(), 0);
			}
        }
        else
        {
            bool targetFound = false;
            for (size_t j = 0; j < clients.size(); j++)
            {
                if(normalize(clients[j].getNickName()) == normalize(target)) //must send to it self her ???
                {
                    textMsg = ":" + curr_client->getClientName() + " PRIVMSG " + target + " :" + cmd[2] +"\r\n";
                    send(this->clients[j].getFD(), textMsg.c_str(), textMsg.size(), 0);
                    targetFound = true;
                }
            }
            if (targetFound == false)
                sendErrorMessage(curr_client, target, " :No such nick", "401");
        }
    }
}


void server::handleJoin(client * currentClient, std::vector<std::string> & command)
{
	std::vector<std::string> keys;
	std::vector <std::string> channels;

	if(!currentClient->isAuthenticat())
	{
		sendErrorMessage(currentClient,"JOIN", " :You have not registered", "451");
		return;
	}
	if(command.size() < 2)
	{
		sendErrorMessage(currentClient,"JOIN", " :Not enough parameters", "461");
		return;
	}
	channels = splitArgument(command[1]);
	if(command.size() >= 3)
		keys = splitArgument(command[2]);

	while (keys.size() < channels.size())
	{
		keys.push_back("");
	}
	for (size_t i = 0; i < channels.size(); i++)
	{
		handleSingleJoin(currentClient, channels[i], keys[i]);
	}

}

void server::handleSingleJoin(client * currentClient,std::string & channelName, std::string &channelKey)
{
	std::string reply;
	std::map<std::string, Channel>::iterator it;
	std::pair<std::map<std::string, Channel>::iterator, bool> result;
	std::string lowerCaseChannelName;
	std::stringstream ss;
	if(channelName.size() == 1 && (channelName[0] == '#' || channelName[0]== '&'))
	{
		sendErrorMessage(currentClient,channelName, " :No such channel", "403");
		return;
	}
	if(!validateChannelName(channelName))
	{
		sendErrorMessage(currentClient,channelName, " :Bad Channel Mask", "476");
		return;
	}
	lowerCaseChannelName = normalize(channelName);
	it = this->Channels.find(lowerCaseChannelName);
	if(it == this->Channels.end())
	{
		result = this->Channels.insert(std::make_pair(lowerCaseChannelName, Channel(channelName)));
		it = result.first;
		it->second.addMember(currentClient->getFD());
		it->second.addOperator(currentClient->getFD());
	}
	else
	{
		if(!checkChannelModes(currentClient, channelName, channelKey, it))
			return;
		it->second.addMember(currentClient->getFD());
		if(it->second.isInvited(currentClient->getFD()))
			it->second.removeInvite(currentClient->getFD());
	}
	reply = ":" + currentClient->getClientName() + " JOIN " + it->second.getChannelName() +"\r\n";
	for (std::set<int>::iterator sit = it->second.getMembers().begin(); sit != it->second.getMembers().end(); sit++)
	{
		send(*sit, reply.c_str(), reply.size(), 0);
	}

	if(!it->second.getTopic().empty())
	{
		reply = ":ircserv 332 " + currentClient->getNickName() + " " + it->second.getChannelName() + " :" + it->second.getTopic() + "\r\n";
		send(currentClient->getFD(), reply.c_str(), reply.size(), 0);
		ss << it->second.getTopicModificationDate();
		reply = ":ircserv 333 " + currentClient->getNickName() + " " + it->second.getChannelName() + " " + it->second.getTopicSetter() + " " + ss.str() + "\r\n";
		send(currentClient->getFD(), reply.c_str(), reply.length(), 0);
	}
	server::broadcastNamesList(currentClient,  it);
}

bool server::checkChannelModes(client *currentClient, std::string &channelName, std::string &channelKey, std::map<std::string, Channel>::iterator &it)
{
	if (it->second.isMember(currentClient->getFD()))
	{
		return false;
	}
	if (it->second.isInviteOnly() && !it->second.isInvited(currentClient->getFD()))
	{
		sendErrorMessage(currentClient, channelName, " :Cannot join channel (+i)", "473");
		return false;
	}
	if (it->second.isLocked() && it->second.getChannelKey() != channelKey)
	{
		sendErrorMessage(currentClient, channelName, " :Cannot join channel (+k)", "475");
		return false;
	}
	if (it->second.isLimited() && it->second.getMembers().size() >= it->second.getMaxLimit())
	{
		sendErrorMessage(currentClient, channelName, " :Cannot join channel (+l)", "471");
		return false;
	}
	return true;
}

void server::handleMode(client * currentClient, std::vector<std::string> &command)
{
	std::map<std::string, Channel>::iterator it;
	std::string channelName;
	std::string modeString;
	std::string appliedModes;
	std::string appliedParams;
	short addOrRemove;
	size_t argIndex;
	size_t oModeCount;

	if(!currentClient->isAuthenticat())
	{
		sendErrorMessage(currentClient,"MODE", " :You have not registered", "451");
		return;
	}
	if(command.size() < 2)
	{
		sendErrorMessage(currentClient,"MODE", " :Not enough parameters", "461");
		return;
	}
	channelName = command[1];
	it = this->Channels.find(normalize(channelName));
	if(it == this->Channels.end())
	{
		sendErrorMessage(currentClient,channelName, " :No such channel", "403");
		return;
	}
	if(command.size() == 2)
	{
		printChannelModes(currentClient, channelName, it);
		return;
	}
	if(!it->second.isOperator(currentClient->getFD()))
	{
		sendErrorMessage(currentClient,channelName, " :You're not channel operator", "482");
		return;
	}
	modeString = command[2];
	addOrRemove = 0;
	argIndex = 3;
	appliedParams = "";
	appliedModes = "";
	oModeCount = 0;
	for (size_t i = 0; i < modeString.length(); i++)
	{
		if(modeString[i] == '+')
		{
			addOrRemove = 1;
			continue;
		}
		else if(modeString[i] == '-')
		{
			addOrRemove = -1;
			continue;
		}
		else if(modeString[i] != 'i' && modeString[i] != 't' && modeString[i] != 'k'
			&& modeString[i] != 'o' && modeString[i] != 'l')
		{
			sendErrorMessage(currentClient, std::string(1,modeString[i]), " :is unknown mode char to me", "472");
			continue;
		}
		if (addOrRemove == 0)
			continue;
		if (((modeString[i] == 'k' || modeString[i] == 'l') && addOrRemove == 1))
		{
			if (argIndex >= command.size())
			{
				sendErrorMessage(currentClient, "MODE"," :Not enough parameters", "461");
				continue;
			}
			if(handleSingleMode(currentClient, modeString[i], addOrRemove, it, command[argIndex], channelName))
			{
				appliedModes += "+" + std::string(1, modeString[i]);
				appliedParams += " " + command[argIndex];
			}
			argIndex++;
		}
		else if (modeString[i] == 'o')
		{
			if (oModeCount >= 3)
			{
				if (argIndex < command.size())
				{
					argIndex++;
					continue;
				}
			}
			if (argIndex >= command.size())
			{
				sendErrorMessage(currentClient, "MODE"," :Not enough parameters", "461");
				continue;
			}
			if(handleSingleMode(currentClient, modeString[i], addOrRemove, it, command[argIndex], channelName))
			{
				if(addOrRemove == 1)
					appliedModes += "+";
				else if(addOrRemove == -1)
					appliedModes += "-";
				appliedModes +=  modeString[i];
				appliedParams += " " + command[argIndex];
			}
			argIndex++;
			oModeCount++;
		}
		else
		{
			if(handleSingleMode(currentClient, modeString[i], addOrRemove, it, "", channelName))
			{
				if(addOrRemove == 1)
					appliedModes += "+";
				else if(addOrRemove == -1)
					appliedModes += "-";
				appliedModes +=  modeString[i];
			}
		}
	}
	if (!appliedModes.empty())
	{
		appliedModes = ":" + currentClient->getClientName() + " MODE " + it->second.getChannelName() + " " + appliedModes +\
						 appliedParams + "\r\n";
		for (std::set<int>::iterator sit = it->second.getMembers().begin(); sit != it->second.getMembers().end(); sit++)
		{
			send(*sit, appliedModes.c_str(), appliedModes.size(), 0);
		}

	}
}

bool server::handleSingleMode(client *currentClient, char mode, short addOrRemove, std::map<std::string, Channel>::iterator it, std::string parameter, std::string channelName)
{
	std::stringstream ss;
	size_t limit;
	std::string reply;

	limit = 0;
	if(mode == 'i')
	{
		if(addOrRemove == 1 && !it->second.isInviteOnly())
		{
			it->second.setInviteOnly(true);
			return true;
		}
		else if(addOrRemove == -1 && it->second.isInviteOnly())
		{
			it->second.setInviteOnly(false);
			return true;
		}
		return false;
	}
	else if(mode == 't')
	{
		if(addOrRemove == 1 && !it->second.isTopicRestricted())
		{
			it->second.setTopicRestriction(true);
			return true;
		}
		else if(addOrRemove == -1 && it->second.isTopicRestricted())
		{
			it->second.setTopicRestriction(false);
			return true;
		}
		return false;
	}
	else if(mode == 'k')
	{
		if(addOrRemove == 1)
		{
			if(it->second.isLocked() && it->second.getChannelKey() == parameter)
			{
				sendErrorMessage(currentClient, it->second.getChannelName(), " :Channel key already set", "467");
				return false;
			}
			it->second.setLocked(true);
			it->second.setChannelKey(parameter);
			return true;
		}
		else if(addOrRemove == -1 && it->second.isLocked())
		{
			it->second.setLocked(false);
			it->second.setChannelKey("");
			return true;
		}
		return false;
	}
	else if(mode == 'l')
	{
		if(addOrRemove == 1)
		{
			for (size_t i = 0; i < parameter.length(); i++)
			{
				if(!std::isdigit(parameter[i]))
				{
					reply += ":ircserv 696 " + currentClient->getNickName() + " " + channelName + " " + std::string (1, mode) + " " + parameter + " :invalid mode parameter\r\n";
					send(currentClient->getFD(), reply.c_str(), reply.length(), 0);
					return false;
				}
			}
			ss << parameter;
			ss >> limit;
			if(limit == 0)
			{
				reply += ":ircserv 696 " + currentClient->getNickName() + " " + channelName + " " + std::string (1, mode) + " " + parameter + " :invalid mode parameter\r\n";
					send(currentClient->getFD(), reply.c_str(), reply.length(), 0);
				return false;
			}
			if (it->second.isLimited() && it->second.getMaxLimit() == limit)
    			return false;
			it->second.setLimitState(true);
			it->second.setMaxLimit(limit);
			return true;
		}
		else if(addOrRemove == -1 && it->second.isLimited())
		{
			it->second.setLimitState(false);
			it->second.setMaxLimit(0);
			return true;
		}
		return false;
	}
	else if (mode == 'o')
	{
		for (size_t i = 0; i < this->clients.size(); i++)
		{
			if (normalize(this->clients[i].getNickName()) == normalize(parameter))
			{
				if (!it->second.isMember(this->clients[i].getFD()))
				{
					sendErrorMessage(currentClient,parameter + " " + channelName, " :They aren't on that channel", "441");
					return false;
				}
				if (addOrRemove == 1 && !it->second.isOperator(this->clients[i].getFD()))
				{
					it->second.addOperator(this->clients[i].getFD());
					return true;
				}
				else if (addOrRemove == -1 && it->second.isOperator(this->clients[i].getFD()))
				{
					it->second.removeOperator(this->clients[i].getFD());
					return true;
				}
				return false;
			}
		}
		sendErrorMessage(currentClient, parameter, " :No such nick", "401");
		return false;
	}
	return false;
}

void printChannelModes(client * currentClient, std::string & channelName, std::map<std::string, Channel>::iterator &it)
{
		std::string response;
		std::stringstream ss;
		std::string args;

		args = "";
		response = ":ircserv 324 " + currentClient->getNickName() \
				+ " " + channelName + " +";
		if(it->second.isInviteOnly())
			response += "i";
		if(it->second.isTopicRestricted())
			response += "t";
		if(it->second.isLocked())
		{
			response += "k";
			args += " " + it->second.getChannelKey();
		}
		if(it->second.isLimited())
		{
			response += "l";
			ss << it->second.getMaxLimit();
			args += " " + ss.str();
		}
		response += args + "\r\n";
		send(currentClient->getFD(), response.c_str(), response.length(), 0);
		ss.str("");
		ss.clear();
		ss << it->second.getCreationTime();
		sendErrorMessage(currentClient, channelName ," " + ss.str(), "329");
		return;
}


void server::broadcastNamesList(client * currentClient, std::map<std::string, Channel>::iterator &it)
{
	std::string reply;
	bool first;

	first = true;
	reply = ":ircserv 353 " + currentClient->getNickName() + " = " + it->second.getChannelName() + " :";
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if(it->second.isMember(this->clients[i].getFD()))
		{
			if(!first)
				reply += " ";

			if(it->second.isOperator(this->clients[i].getFD()))
				reply += "@";
			reply += this->clients[i].getNickName();
			first = false;
		}
	}
	reply += "\r\n";
	send(currentClient->getFD(), reply.c_str(), reply.size(), 0);
	reply = ":ircserv 366 " + currentClient->getNickName() + " " + it->second.getChannelName() \
			+ " :End of /NAMES list\r\n";
	send(currentClient->getFD(), reply.c_str(), reply.size(), 0);
}

void sendErrorMessage(client * currentClient, std::string command, std::string message,std::string errCode)
{
	std::string response;
    std::string target = currentClient->getNickName();
    if (target.empty())
		target = "*";
	response = ":ircserv " + errCode + " " + target + " " + command + message + "\r\n";
	send(currentClient->getFD(), response.c_str(), response.length(), 0);
}

void server::handleTopic(client * currentClient, std::vector<std::string> & command)
{
	std::string response;
	std::map<std::string, Channel>::iterator it;
	std::string channelName;
	std::stringstream ss;

	if(!currentClient->isAuthenticat())
	{
		sendErrorMessage(currentClient,"TOPIC", " :You have not registered", "451");
		return;
	}

	if(command.size() < 2)
	{
		sendErrorMessage(currentClient,command[0], " :Not enough parameters", "461");
		return;
	}
	channelName = normalize(command[1]);
	it = this->Channels.find(channelName);
	if(it == this->Channels.end())
	{
		sendErrorMessage(currentClient,channelName, " :No such channel", "403");
		return;
	}
	if(!it->second.isMember(currentClient->getFD()))
	{
		sendErrorMessage(currentClient,channelName, " :You're not on that channel", "442");
		return;
	}
	if(command.size() > 2)
	{
		server::manageTopic(currentClient, command, it);
		return;
	}
	if(it->second.getTopic().empty())
	{
		sendErrorMessage(currentClient, channelName, " :No topic is set", "331");
		return;
	}
	response = ":ircserv 332 " + currentClient->getNickName() + " " + it->second.getChannelName() + " :" + it->second.getTopic() + "\r\n";
	send(currentClient->getFD(), response.c_str(), response.length(), 0);
	ss << it->second.getTopicModificationDate();
	response =":ircserv 333 " + currentClient->getNickName() + " " + it->second.getChannelName() + " " + it->second.getTopicSetter() + " "
	+ ss.str() +"\r\n";
	send(currentClient->getFD(), response.c_str(), response.length(), 0);
}

void server::manageTopic(client * currentClient, std::vector<std::string> & command, std::map<std::string, Channel>::iterator & it)
{
	std::string channelName;
	std::string topicMessage;
	std::string topicSetter;
	std::string reply;

	channelName = command[1];
	if(it->second.isTopicRestricted() && !it->second.isOperator(currentClient->getFD()))
	{
		sendErrorMessage(currentClient, channelName, " :You're not channel operator", "482");
		return;
	}
	topicMessage = command[2];
	it->second.setTopic(topicMessage);
	if(!topicMessage.empty())
	{
		topicSetter = currentClient->getClientName() ;
		it->second.setTopicSetter(topicSetter);
		it->second.setTopicModificationDate(time(NULL));
	}
	else
	{
		it->second.setTopicSetter("");
		it->second.setTopicModificationDate(0);
	}
	reply = ":" + currentClient->getClientName()+ " TOPIC " + it->second.getChannelName() + " :" + topicMessage + "\r\n";
	for (std::set<int>::iterator sit = it->second.getMembers().begin(); sit !=  it->second.getMembers().end(); sit++)
	{
		send(*sit, reply.c_str(), reply.length(), 0);
	}

}