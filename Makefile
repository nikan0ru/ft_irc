CPP=c++

CPPFLAGS= -Wall -Wextra 
RM= rm -rf

SRC= main.cpp

OBJ= $(SRC:.cpp=.o)

NAME= ircserv

all: $(NAME)

$(NAME):$(OBJ)
	$(CPP) $(CPPFLAGS)  -o $@ $^

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean : clean
	$(RM) $(NAME)

re: fclean all
