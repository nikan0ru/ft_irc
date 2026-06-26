#ifndef CHANNEL_HPP
#define	CHANNEL_HPP


#include <string>
#include <vector>
#include "client.hpp"

class Channel
{
private:
	std::string name;
	int memberCount;
	std::vector<client *> members;
	std::vector<client *> operators;

public:
	Channel(std::string n): name(n), memberCount(0)
	{

	}
	std::vector<client *> & getMembers()
	{
		return this->members;
	}
	void addMember(client * clnt)
	{
		if(this->memberCount == 0)
			this->operators.push_back(clnt);
		this->members.push_back(clnt);
		memberCount++;
	}
	bool isOperator(client &clnt)
	{
		for (size_t i = 0; i < this->operators.size(); i++)
		{
			if (this->operators.at(i) == &clnt)
				return true;
		}
		return false;
	}

};
#endif
