
#include "../includes/server.hpp"

server::server(const std::string& portnum, const std::string& authpass):socket_fd(-1), reuse_flag(1),
                                                                        servport(portnum),servpass(authpass), client_fd(-1)

{
}


int server::creat_sokect()
{
    struct addrinfo hints, *serverinfo;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if (getaddrinfo(NULL, this->servport.c_str(), &hints, &serverinfo) != 0)
    {
        std::cout << "ERROR: getaddrinfo failed.\n";
        return EXIT_FAILURE;
    }
    this->socket_fd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (this->socket_fd < 0)
    {
        std::cout << "ERROR: creation of socket failed.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &this->reuse_flag, sizeof(this->reuse_flag)) == -1)
    {
        std::cout << "ERROR: failed to reuse socket.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    // " You probably noticed that when you run listener, above, it just sits there until a packet arrives.
    // What happened is that it called recvfrom(), there was no data, and so recvfrom() is said to “block”
    // (that is, sleep there) until some data arrives.

    // Lots of functions block. accept() blocks. All the recv() functions block.
    // The reason they can do this is because they’re allowed to.
    // When you first create the socket descriptor with socket(),
    // the kernel sets it to blocking. If you don’t want a socket to be blocking,
    // you have to make a call to fcntl() "

    if (fcntl(this->socket_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "ERROR: failed to set NONBLOCK for socket fd.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    // "Normally, recv() on a blocking socket blocks — your program just sits there frozen until data arrives.
    // Once you set O_NONBLOCK, recv() (or read()) refuses to wait.
    // If there's no data, it returns immediately with -1,
    // and sets errno to EAGAIN or EWOULDBLOCK (which one depends on the OS/libc)"

    if (bind(this->socket_fd, serverinfo->ai_addr, serverinfo->ai_addrlen) ==  -1)
    {
        std::cout << "ERROR: bind the socket failed. " << "\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }
    freeaddrinfo(serverinfo);
    return EXIT_SUCCESS;
}

void server::closeAllFds()
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client (" << clients[i].getFD() << ") Disconnected\n";
        close(clients[i].getFD());
    }
    if (this->socket_fd != -1)
    {
        std::cout << "Server (" << this->socket_fd << ") Disconnected\n";
        close(this->socket_fd);
    }
}


void server::removeClient(int fd)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].getFD() == fd)
            clients.erase(clients.begin() + i);
    }
}


void server::removeFd(int fd)
{
    for (size_t i = 0;i < pollfds.size(); i++)
    {
        if (pollfds[i].fd == fd)
            pollfds.erase(pollfds.begin() + i);
    }
}



int server::listen_and_monitorfdstatus()
{
    // second arg in listen It's the size of a small holding queue for connections that have finished
    // the TCP handshake but that your program hasn't called accept() on yet using sockaddr_storage.
    // The moment you accept() one,
    // it leaves that queue — it doesn't count against the limit anymore.
    /*
        * Maximum queue length specifiable by listen.
        #define SOMAXCONN       128
    */
    if (listen(this->socket_fd, SOMAXCONN) == -1)
    {
        std::cout << "ERROR: Failed to start listening for incoming connections.\n";
        return EXIT_FAILURE;
    }
    // may be i need to handel signals here

    struct pollfd Newpollfd;

    Newpollfd.fd = this->socket_fd;
    Newpollfd.events = POLLIN;
    Newpollfd.revents = 0; //i can use it or not ??

    this->pollfds.push_back(Newpollfd);

    std::cout << "server: waiting for connections...\n";
    while (1)
    {
        if (poll(&pollfds[0], pollfds.size(), -1) == -1)
        {
            std::cout << "pool failed";
            return EXIT_FAILURE;
        }
        this->procces_connections();
    }
    closeAllFds();
    return EXIT_SUCCESS;
}



int server::procces_connections()
{
    for (size_t i = 0; i < pollfds.size(); i++)
    {
        if (pollfds[i].revents & POLLIN)
        {
            if (pollfds[i].fd == this->socket_fd)
                this->acceptNewClient();
            else
                this->handelNewData(pollfds[i].fd);
        }
    }
    return EXIT_SUCCESS;
}

int server::acceptNewClient()
{
    struct sockaddr_storage newCli_inf;
    client newClient;
    socklen_t NewCli_sockaddr_size;

    NewCli_sockaddr_size = sizeof(newCli_inf);
    this->client_fd = accept(this->socket_fd, (struct sockaddr*)&newCli_inf, &NewCli_sockaddr_size);

    if (this->client_fd == -1)
    {
        std::cout << "ERROR: accept clinet socket failed.\n";
        return EXIT_FAILURE;
    }

    if (fcntl(this->client_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "ERROR: failed to set NONBLOCK for socket fd.\n";
        close(this->client_fd);
        return EXIT_FAILURE;
    }

    struct pollfd temp_pfd;
    temp_pfd.fd = this->client_fd;
    temp_pfd.events = POLLIN;
    temp_pfd.revents = 0;

    struct sockaddr_in *ipv4 = (struct sockaddr_in*)&newCli_inf;
    newClient.setFD(this->client_fd);
    newClient.setIpAdd(inet_ntoa(ipv4->sin_addr));

    this->clients.push_back(newClient);
    this->pollfds.push_back(temp_pfd);
	std::cout << "Client <" << this->client_fd << "> Connected\n";

    return EXIT_SUCCESS;
}

client *server::getClient(int fd)
{
    for (size_t i = 0; i < this->clients.size(); i++)
    {
        if (this->clients[i].getFD() == fd)
            return &clients[i];
    }
    return NULL;
}

std::vector<std::string> server::split_recved_buffer(std::string buff)
{
    std::istringstream text(buff);
    std::string token;
    std::vector<std::string> cmds;
    while (std::getline(text, token))
    {
        size_t cpos = token.find_first_of("\n\r");
        if (cpos != std::string::npos)
            token = token.substr(0, cpos);
        cmds.push_back(token);
    }
    return cmds;
}

std::vector<std::string> server::splited_cmd(std::string& cmd)
{
    std::istringstream msg(cmd);
    std::vector<std::string> vec;
    std::string word;
    // int i = 0;
    while (msg >> word)
        vec.push_back(word);
    return vec;
}

void server::parse_and_exe(client *curClient, std::vector<std::string> splited_cmd)
{
	std::string Command;

	if(splited_cmd.empty())
		return;
	Command = splited_cmd[0];
	for (size_t i = 0; i < Command.length(); i++)
	{
		Command[i] = std::toupper(Command[i]);
	}
    if((!Command.compare("USER") || !Command.compare("NICK") || !Command.compare("PASS")))
        handelAuthentication(curClient, splited_cmd);
	else if(!Command.compare("JOIN"))
		handleJoin(curClient, splited_cmd);
	else if(!Command.compare("TOPIC"))
		handleTopic(curClient, splited_cmd);
	else if(!Command.compare("MODE"))
		handleMode(curClient, splited_cmd);
	else
		sendErrorMessage(curClient, Command, " :Unknown command", "421");

};

bool server::isValidNickName(std::string& nickName)
{
    if (isdigit(nickName[0]) || nickName[0] == '&' || nickName[0] == '#' || nickName[0] == ':')
                    return false;
    size_t nicksize = nickName.size();
    for (size_t i = 0; i < nicksize; i++)
    {
        if (!isdigit(nickName[i]) && !isalpha(nickName[i]) && nickName[i] != '[' && nickName[i] != ']' && nickName[i] != '{'
                    && nickName[i] != '}' && nickName[i] != '\\' && nickName[i] != '|')
                    return false;
    }
    return true;
}

void server::handelAuthentication(client* curr_client, std::vector<std::string>& cmd)
{
    int cmdsize = cmd.size();
    if (!cmd[0].compare("PASS"))
    {
        if (curr_client->isAuthenticat())
            return (std::cout << "ERR_ALREADYREGISTRED (462)\n", void());
        if (cmdsize != 2)
            return (std::cout << "PASS: ERR_NEEDMOREPARAMS (461)\n", void()); // what if the pasword contiene spaces
        if (cmd[1].compare(this->servpass))
            return (std::cout << "ERR_PASSWDMISMATCH (464)\n", curr_client->setPassStatusFalse(), void());
        curr_client->setAuthenRequirment(1);
    }
    else if (!cmd[0].compare("NICK"))
    {
        if (cmdsize != 2)
            return (std::cout << "NICK: ERR_NONICKNAMEGIVEN (431)\n", void());
        if (!isValidNickName(cmd[1]))
        {
            return(std::cout << "NICK: ERR_ERRONEUSNICKNAME (432)\n", void());
        }
        for (size_t i = 0 ; i < this->clients.size(); i++)
        {
            if (clients[i].getNickName() == cmd[1] && clients[i].getFD() != curr_client->getFD())
                return (std::cout << "NICK: ERR_NICKNAMEINUSE (433)\n", void());
        }
        curr_client->setNickName(cmd[1]);
        curr_client->setAuthenRequirment(2);
    }
    else
    {
        if (cmdsize != 5)
            return (std::cout << "USER: ERR_NEEDMOREPARAMS (461)\n", void());
        if (curr_client->isAuthenticat())
            return (std::cout << "USER: ERR_ALREADYREGISTRED (462)\n", void()); // user parse
        curr_client->setUserName(cmd[1]);
        curr_client->setAuthenRequirment(3);
    }
    if (curr_client->checkAuthenRequirment() == true && curr_client->isAuthenticat() == false)
        curr_client->setAsAuthenticated();
    return;
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
	lowerCaseChannelName = toLowerCase(channelName);
	it = this->Channels.find(lowerCaseChannelName);
	if(it == this->Channels.end())
	{
		result = this->Channels.insert(std::make_pair(lowerCaseChannelName, Channel(lowerCaseChannelName)));
		std::cout << "Channel created successfully!!!!! deleteme"<< std::endl;
		it = result.first;
		it->second.addMember(currentClient->getFD());
		it->second.addOperator(currentClient->getFD());
	}
	else
	{
		if(!checkChannelModes(currentClient, lowerCaseChannelName, channelKey, it))
			return;
		it->second.addMember(currentClient->getFD());
		if(it->second.isInvited(currentClient->getFD()))
			it->second.removeInvite(currentClient->getFD());
	}
	reply = ":" +currentClient->getNickName() + "!" + currentClient->getUserName() + "@"
	+ currentClient->getIpAdd() + " JOIN " + channelName +"\r\n";
	for (unsigned long i = 0; i < this->clients.size(); i++)
	{
		if(it->second.isMember(this->clients[i].getFD()))
		{
			send(this->clients[i].getFD(), reply.c_str(), reply.size(), 0);
		}
	}

	if(!it->second.getTopic().empty())
	{
		reply = ":ircserv 332 " + currentClient->getNickName() + " "
				+ channelName + " :" + it->second.getTopic() + "\r\n";
		send(currentClient->getFD(), reply.c_str(), reply.size(), 0);
	}
	server::broadcastNamesList(currentClient,channelName,  it);
}

bool server::checkChannelModes(client *currentClient, std::string &channelName, std::string &channelKey, std::map<std::string, Channel>::iterator &it)
{
	if (it->second.isMember(currentClient->getFD()))
	{
		std::cout << "is already member of this channel delete me" << std::endl;
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
	std::string lowerCaseChannelName;
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
	lowerCaseChannelName = toLowerCase(channelName);
	it = this->Channels.find(lowerCaseChannelName);
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
				continue;
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
				continue;
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
		appliedModes = ":" + currentClient->getNickName() + "!" + currentClient->getUserName() +\
						"@" + currentClient->getIpAdd() + " MODE " + channelName + " " + appliedModes +\
						 appliedParams + "\r\n";
		for (size_t i = 0; i < this->clients.size(); i++)
		{
			if (it->second.isMember(this->clients[i].getFD()))
			{
				send(this->clients[i].getFD(), appliedModes.c_str(), appliedModes.size(), 0);
			}
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
		else if(addOrRemove == -1)
		{
			it->second.setInviteOnly(false);
			return true;
		}
		return false;
	}
	else if(mode == 't')
	{
		if(addOrRemove == 1)
			it->second.setTopicRestriction(true);
		else if(addOrRemove == -1)
			it->second.setTopicRestriction(false);
		return true;
	}
	else if(mode == 'k')
	{
		if(addOrRemove == 1)
		{
			it->second.setLocked(true);
			it->second.setChannelKey(parameter);
		}
		else if(addOrRemove == -1)
		{
			it->second.setLocked(false);
			it->second.setChannelKey("");
		}
		return true;
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
			it->second.setLimitState(true);
			it->second.setMaxLimit(limit);
		}
		else if(addOrRemove == -1)
		{
			it->second.setLimitState(false);
			it->second.setMaxLimit(0);
		}
		return true;
	}
	else if (mode == 'o')
	{
		for (size_t i = 0; i < this->clients.size(); i++)
		{
			if (this->clients[i].getNickName() == parameter)
			{
				if (!it->second.isMember(this->clients[i].getFD()))
				{
					sendErrorMessage(currentClient,parameter + " " + channelName, " :They aren't on that channel", "441");
					return false;
				}
				if (addOrRemove == 1)
					it->second.addOperator(this->clients[i].getFD());
				else if (addOrRemove == -1)
					it->second.removeOperator(this->clients[i].getFD());
				return true;
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


void server::broadcastNamesList(client * currentClient,std::string &channelName, std::map<std::string, Channel>::iterator &it)
{
	std::string reply;
	bool first;

	first = true;
	reply = ":ircserv 353 " + currentClient->getNickName() + " = " + channelName + " :";
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
	reply = ":ircserv 366 " + currentClient->getNickName() + " " + channelName \
			+ " :End of /NAMES list\r\n";
	send(currentClient->getFD(), reply.c_str(), reply.size(), 0);
}

void sendErrorMessage(client * currentClient, std::string command, std::string message,std::string errCode)
{
	std::string response;

	response = ":ircserv " + errCode + " " +currentClient->getNickName() \
				+ " " + command + message + "\r\n";
	send(currentClient->getFD(), response.c_str(), response.length(), 0);
}

void server::handleTopic(client * currentClient, std::vector<std::string> & command)
{
	std::string response;
	std::map<std::string, Channel>::iterator it;

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
	response = currentClient->getUserName() + " " + command[1];
	it = this->Channels.find(command[1]);
	if(it == this->Channels.end())
	{
		sendErrorMessage(currentClient,command[0], " :No such channel", "403");
		return;
	}
	if(!it->second.isMember(currentClient->getFD()))
	{
		sendErrorMessage(currentClient,command[0], " :You're not on that channel", "442");
		return;
	}
	if(command.size() > 2)
	{
		server::manageTopic(currentClient, command);
		return;
	}
	if(it->second.getTopic().empty())
	{
		sendErrorMessage(currentClient,command[0], " :No topic is set", "331");
		return;
	}
	response = currentClient->getNickName() + "?? " + command[1] + it->second.getTopic() + "\r\n";
	send(currentClient->getFD(), response.c_str(), response.length(), 0);
}

void server::manageTopic(client * curr_client, std::vector<std::string> & command)
{
	std::string newTopic;
	(void)(command);
	(void)(curr_client);

}


int server::handelNewData(int cliFd)
{
    char buffer[1024];
    std::memset(buffer, 0, 1024);
    int bytes = recv(cliFd, buffer, sizeof(buffer) -1, 0);
    std::vector<std::string> msg;
    client *currClient = getClient(cliFd);
    if (bytes <= 0)
    {
        if (bytes == -1)
        {
            std::cout << "no messages are available at the socket (maybe ctrlc or ctr..)\n";
            return EXIT_SUCCESS;
        }
        std::cout << "he peer has performed an orderly shutdown.\n";
        removeClient(cliFd);
        removeFd(cliFd);
        close(cliFd);
    }
    else
    {
        std::cout << "msg recved\n";
        currClient->clientSetBuff(split_recved_buffer(buffer));
        msg = currClient->clientGetBuff();
        for (size_t i =0; i < msg.size(); i++)
        {
            if (!msg[i].empty())
                parse_and_exe(currClient, splited_cmd(msg[i]));
        }
    }
    return EXIT_SUCCESS;
}

server::~server(){};
