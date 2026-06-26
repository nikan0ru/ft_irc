#ifndef CHANNEL_HPP
#define	CHANNEL_HPP
#include <string>
#include <vector>
#include "client.hpp"

class Channel
{
private:
	std::string name;
	std::vector<client *> members;
	std::vector<client *> operators;

public:
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
