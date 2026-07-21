
#include "../includes/server.hpp"

bool g_running = true;

void signalHandler(int signum)
{
    (void)signum;
    g_running = false;
}

std::string normalize(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		str[i] = std::tolower(str[i]);
		if (str[i] == '[')
			str[i] = '{';
		if (str[i] == ']')
			str[i] = '}';
		if (str[i] == '\\')
			str[i] = '|';
	}
	return str;
}

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
		{
            clients.erase(clients.begin() + i);
			break;
		}
    }
	for(std::map<std::string, Channel>::iterator it = this->Channels.begin(); it != this->Channels.end(); it++)
	{
		if(it->second.isMember(fd))
			it->second.removeMember(fd);
		if(it->second.isOperator(fd))
			it->second.removeOperator(fd);
		if(it->second.isInvited(fd))
			it->second.removeInvite(fd);
		if(it->second.getMembers().empty())
			this->Channels.erase(it++);
		else
			it++;
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
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGPIPE, SIG_IGN);
    std::cout << "server: waiting for connections...\n";
    while (g_running)
    {
        if (poll(&pollfds[0], pollfds.size(), -1) == -1)
        {
            if (errno == EINTR)
                break;
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
    while (msg >> word)
    {
        if (word[0] == ':')
        {
            std::string lastword;
            word = word.substr(1);
            std::getline(msg, lastword);
            if (lastword.empty())
                vec.push_back(word);
            else
                vec.push_back(word + lastword);
            break;
        }
        vec.push_back(word);
    }
    return vec;
}

void server::parse_and_exe(client *curClient, std::vector<std::string> splited_cmd)
{
	std::string Command;
	if(splited_cmd.empty())
		return;
	splited_cmd[0] = normalize(splited_cmd[0]);
	Command = splited_cmd[0];
    if((!Command.compare("user") || !Command.compare("nick") || !Command.compare("pass")))
        handleAuthentication(curClient, splited_cmd);
	else if(!Command.compare("join"))
		handleJoin(curClient, splited_cmd);
	else if(!Command.compare("topic"))
		handleTopic(curClient, splited_cmd);
	else if(!Command.compare("mode"))
		handleMode(curClient, splited_cmd);
  else if(!Command.compare("privmsg"))
		handlePrivmsg(curClient, splited_cmd);
  else if(!Command.compare("invite"))
		handleInvite(curClient, splited_cmd);
	else if(!Command.compare("kick"))
		handleKick(curClient, splited_cmd);
	else
		sendErrorMessage(curClient, Command, " :Unknown command", "421");

};

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
					sendErrorMessage(currentClient, nickList[i] + " " + it->second.getChannelName(), " :They aren't on that channel", "441");
					break;
				}
				for (size_t k = 0; k < this->clients.size(); k++)
				{
					if (it->second.isMember(this->clients[k].getFD()))
					{
						if (cmd.size() > 3)
							kickReason = cmd[3];
						else
							kickReason = nickList[i];
						message = ":" + currentClient->getClientName() + " KICK " + it->second.getChannelName() + " " + nickList[i] + " :" + kickReason + "\r\n";
						send(this->clients[k].getFD(), message.c_str(), message.length(), 0);
					} // check my optimization
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

bool server::isValidNickName(std::string& nickName)
{
    size_t nicksize = nickName.size();
    if (isdigit(nickName[0]) || nickName[0] == '&' || nickName[0] == '#' || nickName[0] == ':' || nicksize > 9)
                    return false;
    for (size_t i = 0; i < nicksize; i++)
    {
        if (!isdigit(nickName[i]) && !isalpha(nickName[i]) && nickName[i] != '[' && nickName[i] != ']' && nickName[i] != '{'
                    && nickName[i] != '}' && nickName[i] != '\\' && nickName[i] != '`' && nickName[i] != '^' && nickName[i] != '-' && nickName[i] != '|')
                    return false;
    }
    return true;
}


void server::handleAuthentication(client* curr_client, std::vector<std::string>& cmd)
{
	std::string Command;

	Command = cmd[0];
    int cmdsize = cmd.size();
    if (!Command.compare("pass"))
    {
        if (curr_client->isAuthenticat())
            return (sendErrorMessage(curr_client, "PASS", " :You may not reregister", "462"), void());
        if (cmdsize < 2)
            return (sendErrorMessage(curr_client, "PASS", " :Not enough parameters", "461"), void()); // what if the pasword contiene spaces
        if (cmd[1].compare(this->servpass))
            return (sendErrorMessage(curr_client, "PASS", " :Password incorrect", "464"), curr_client->setPassStatusFalse(), void());
        curr_client->setAuthenRequirment(1);
    }
    else if (!Command.compare("nick"))
    {
        if (cmdsize < 2 || cmd[1].empty())
            return (sendErrorMessage(curr_client, "NICK", " :No nickname given", "431"), void());
        if (!isValidNickName(cmd[1]))
            return(sendErrorMessage(curr_client, "NICK", " :Erroneus nickname", "432"), void());
        for (size_t i = 0 ; i < this->clients.size(); i++)
        {
            if (normalize(clients[i].getNickName()) == normalize(cmd[1]) && clients[i].getFD() != curr_client->getFD())
                return (sendErrorMessage(curr_client, "NICK", " :Nickname is already in use", "433"), void());
        }
        curr_client->setNickName(cmd[1]);
        curr_client->setAuthenRequirment(2);
    }
    else
    {
        if (curr_client->isAuthenticat())
            return (sendErrorMessage(curr_client, "USER", " :You may not reregister", "462"), void());
        if (cmdsize < 5 || cmd[1].empty())
            return (sendErrorMessage(curr_client, "USER", " :Not enough parameters", "461"), void());
        curr_client->setUserName(cmd[1]);
        curr_client->setRealName(cmd[4]);
        curr_client->setAuthenRequirment(3);
    }
    std::string RPL_WELCOME = ":ircserv 001 "+curr_client->getNickName()\
    +" :Welcome to the Internet Relay Network " + curr_client->getClientName() + "\r\n";
    if (curr_client->checkAuthenRequirment() == true && curr_client->isAuthenticat() == false)
        return (curr_client->setAsAuthenticated(), 	send(curr_client->getFD(), RPL_WELCOME.c_str(), RPL_WELCOME.length(), 0), void());
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
