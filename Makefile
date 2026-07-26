NAME = ircserv

SRCS = ./srcs/main.cpp ./srcs/server.cpp ./srcs/client.cpp ./srcs/channel.cpp ./srcs/commands.cpp

INCLUDES = ./includes/channel.hpp ./includes/client.hpp ./includes/server.hpp

FLAGS = -Wall -Wextra -Werror -std=c++98

OBJS = $(SRCS:.cpp=.o)

CXX = c++

RM = rm -rf

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp $(INCLUDES)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re test
