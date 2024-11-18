NAME = Ircserv
CPPFLAGS =-std=c++17 -MMD -MP

SRCNAME = main FDMatcher IRCServer Messages/IRCMessageParse Messages/ServerMessage Messages/tools Messages/Response Messages/rpl command/cmd_topic command/cmd_privmsg command/cmd_mode command/cmd_invite command/cmd_kick command/cmd_quit
SRC     = $(addsuffix .cpp, $(SRCNAME))
OBJ     = $(addsuffix .o, $(SRCNAME))
DEP     = $(addsuffix .d, $(SRCNAME))

all : $(NAME)
$(NAME) : $(OBJ)
	$(CXX) $(CPPFLAGS) $(OBJ) -o $@

-include $(DEP)

%.o : %.c
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJ) $(DEP)

fclean :
	rm -rf $(OBJ) $(DEP)
	rm -rf $(NAME)

re : fclean all

.PHONY : all clean fclean re