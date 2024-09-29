/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihwan <sihwan@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 14:16:22 by sihkang           #+#    #+#             */
/*   Updated: 2024/06/30 12:04:02 by sihwan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tools.hpp"
#include "Response.hpp"

bool isCorrectPassword(serverInfo &info, std::string &client_pw)
{
	return (info.server_pwd == client_pw) ? true : false;
}

bool isCommand(IRCMessage msg, std::string cmd)
{
	if (msg.command.find(cmd) != std::string::npos && msg.command.size() == cmd.size())
		return (true);
	return (false);
}

bool isValidNick(std::string nick)
{
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (nick[i] != '_' && !isalnum(nick[i]))
			return (false);
	}
	return (true);
}

bool numParamCheck(int client_fd, IRCMessage message, int num)
{
	if (message.numParams < num)
	{
		Response::send_message(client_fd, ":dokang 461 " + message.command + " :Need more parameters\r\n");
		return (false);
	}
	return (true);
}

std::string getMessageParams(IRCMessage message)
{
	std::string params = message.params[0];

	for (int i = 1; i < message.numParams; i++)
	{
		params += " " + message.params[i];
	}
	return (params);
}

std::string aftercolonConcat(IRCMessage message)
{
	std::string concatString = "";
	int i;
	for (i = 0; i < message.numParams; i++)
	{
		if (message.params[i].front() == ':')
			break;
	}
	if (i == message.numParams)
		return concatString;
	concatString += message.params[i++].erase(0, 1);
	for (;i < message.numParams; i++)
	{
		concatString += (' ' + message.params[i]);
	}
	return concatString;
}

User& findUser(serverInfo &info, std::string nick)
{
	std::list<User>::iterator it;
	
	for (it = info.usersInServer.begin(); it != info.usersInServer.end(); ++it)
	{
		if ((*it).nick == nick)
			return (*it);
	}
	return (*info.usersInServer.begin());
}

User& findUser(serverInfo &info, int client_fd)
{
	std::list<User>::iterator it;
	
	for (it = info.usersInServer.begin(); it != info.usersInServer.end(); ++it)
	{
		if ((*it).client_fd == client_fd)
			return (*it);
	}
	return (*info.usersInServer.begin());
}

User& findUser(Channel &ch, int client_fd)
{
	std::list<User>::iterator it;
	
	for (it = ch.channelUser.begin(); it != ch.channelUser.end(); ++it)
	{
		if ((*it).client_fd == client_fd)
			return (*it);
	}
	return (*ch.channelUser.begin());
}

User& findUser(Channel &ch, std::string nick)
{
	std::list<User>::iterator it;
	
	for (it = ch.channelUser.begin(); it != ch.channelUser.end(); ++it)
	{
		if ((*it).nick == nick)
			return (*it);
	}
	return (*ch.channelUser.begin());
}


User& findOPUser(Channel &ch, int client_fd)
{
	std::list<User>::iterator it;
	
	for (it = ch.operator_user.begin(); it != ch.operator_user.end(); ++it)
	{
		if ((*it).client_fd == client_fd)
			return (*it);
	}
	return (*ch.operator_user.begin());
}

User& findOPUser(Channel &ch, std::string nick)
{
	std::list<User>::iterator it;
	
	for (it = ch.operator_user.begin(); it != ch.operator_user.end(); ++it)
	{
		if ((*it).nick == nick)
			return (*it);
	}
	return (*ch.operator_user.begin());
}

Channel& findChannel(serverInfo &info, std::string chName)
{
	std::list<Channel>::iterator it;

	for (it = info.channelInServer.begin(); it != info.channelInServer.end(); ++it)
	{
		if ((*it).name == chName)
		{
			return (*it);
		}
	}
	return (*info.channelInServer.begin());
}

std::string channelUserList(Channel &requestedChannel)
{
	std::string userList = "";
	std::list<User>::iterator it;
	for (it = ++(requestedChannel.channelUser.begin()); it != requestedChannel.channelUser.end();)
	{
		if (findOPUser(requestedChannel, (*it).client_fd).nick != "")
		{
			userList += "@";
		}
		userList += (*it).nick;
		if (++it != requestedChannel.channelUser.end())
			userList += " ";
	}
	return (userList);
}

void setChannelMode(Channel &ch, bool i, bool t, bool k, bool o, bool l)
{
	ch.opt[MODE_i] = i;
	ch.opt[MODE_t] = t;
	ch.opt[MODE_k] = k;
	ch.opt[MODE_o] = o;
	ch.opt[MODE_l] = l;
}

std::string getChannelMode(Channel &ch)
{
	std::string setting = "+";
	if (ch.opt[MODE_i] == true)
		setting += "i";
	if (ch.opt[MODE_t] == true)
		setting += "t";
	if (ch.opt[MODE_k] == true)
		setting += "k";
	if (ch.opt[MODE_l] == true)
		setting += "l";
	return (setting);
}

void changeChannelMode(int client_fd, Channel &ch, IRCMessage msg)
{
	if (msg.params[1][0] == '+')
	{
		modifyChannelOpt(client_fd, ch, msg);
	}
	else if (msg.params[1][0] == '-')
	{
		unsetChannelOpt(client_fd, ch, msg);
	}
	else
	{
		Response::send_message(client_fd, "dokang 401 " + (findUser(ch, client_fd)).nick
							+ msg.params[1][0] + " :No such nick");
	}
}

void modifyChannelOpt(int client_fd, Channel &ch, IRCMessage msg)
{
	std::string setting = msg.params[1];
	int arguIdx = 2;
	User &user = findUser(ch, client_fd);
	std::string validOption = '#' + msg.params[0] + " +";
	std::string validArgument = "";
	
	for (size_t i = 1; i < msg.params[1].size(); i++)
	{
		if (setting[i] == 'i')
			ch.opt[MODE_i] = true;
		else if (setting[i] == 't')
			ch.opt[MODE_t] = true;
		else if (setting[i] == 'k')
		{
			if (msg.numParams >= 3)
			{
				ch.key = msg.params[arguIdx++];
				ch.opt[MODE_k] = true;
			}
			else
			{
				Response::send_message(client_fd, "dokang 696 " + user.nick + " #" + ch.name 
									+ " k * :You must specify a parameter for the key mode. Syntax: <key>.\r\n");
				continue;
			}
		}
		else if (setting[i] == 'o')
		{
			User &getPrivilegeUser = findUser(ch, msg.params[arguIdx++]);
			if (getPrivilegeUser.nick == "")
			{
				Response::rpl401_modeErr(client_fd, user, msg.params[arguIdx - 1]);
				continue ;
			}
			
			if (findOPUser(ch, client_fd).nick == "")
			{
				Response::rpl482(client_fd, user, ch.name);
				continue ;
			}

			if (findOPUser(ch, getPrivilegeUser.client_fd).nick == "")
				ch.operator_user.push_back(getPrivilegeUser);
			ch.opt[MODE_o] = true;
		}
		else if (setting[i] == 'l')
		{
			ch.user_limit = atoi(msg.params[arguIdx++].c_str());
			if (ch.user_limit > 0 && ch.user_limit < 100)
				ch.opt[MODE_l] = true;
			else
				continue;
		}
		else
		{
			Response::rpl472(client_fd, user, setting[i]);
			continue;
		}
		
		validOption += setting[i];
		if (setting[i] == 'k' || setting[i] == 'o' || setting[i] == 'l')
		{
			validArgument += msg.params[arguIdx - 1];
			if (arguIdx <= msg.numParams)
				validArgument += " ";
		}
	}

	if (validOption != '#' + msg.params[0] + " +")
	{
		for (std::list<User>::iterator it = ++(ch.channelUser.begin()); it != ch.channelUser.end(); ++it)
		{
			Response::userPrefix(user, (*it).client_fd);
			Response::send_message((*it).client_fd, " " + msg.command + " " + validOption + " " + validArgument + "\r\n");
		}
	}
}

void unsetChannelOpt(int client_fd, Channel &ch, IRCMessage msg)
{
	std::string setting = msg.params[1];
	int arguIdx = 2;
	User &user = findUser(ch, client_fd);
	std::string validOption = '#' + msg.params[0] + " -";
	std::string validArgument = "";
	
	for (size_t i = 1; i < msg.params[1].size(); i++)
	{
		if (setting[i] == 'i')
			ch.opt[MODE_i] = false;
		else if (setting[i] == 't')
			ch.opt[MODE_t] = false;
		else if (setting[i] == 'k')
		{
			if (msg.numParams >= 3 && ch.key == msg.params[arguIdx++])
			{
				ch.opt[MODE_k] = false;
				ch.key = "";
			}
			else
			{
				Response::send_message(client_fd, "dokang 696 " + user.nick + " #" + ch.name 
									+ " k * :You must specify a parameter for the key mode. Syntax: <key>.\r\n");
				continue;
			}
		}
		else if (setting[i] == 'o')
		{
			if (msg.numParams < 3)
				continue;
				
			User &unsetPrivilegeUser = findUser(ch, msg.params[arguIdx++]);
			if (unsetPrivilegeUser.nick == "")
			{
				Response::send_message(client_fd, "dokang 401 " + (findUser(ch, client_fd)).nick
								+ " " + msg.params[arguIdx - 1] + " :No such nick\r\n");
				continue;
			}
			
			ch.opt[MODE_o] = false;
			for (std::list<User>::iterator it = ch.operator_user.begin(); it != ch.operator_user.end(); ++it)
			{
				if ((*it).nick == unsetPrivilegeUser.nick)
				{
					ch.operator_user.erase(it);
					break ;
				}
			}
		}
		else if (setting[i] == 'l')
		{
			ch.opt[MODE_l] = false;
			ch.user_limit = 9999;
		}
		else
		{
			Response::rpl472(client_fd, user, setting[i]);
		}

		validOption += setting[i];
		if (setting[i] == 'k' || setting[i] == 'o')
		{
			validArgument += msg.params[arguIdx - 1];
			if (arguIdx <= msg.numParams)
				validArgument += " ";
		}
	}

	if (validOption != '#' + msg.params[0] + " -")
	{
		for (std::list<User>::iterator it = ++(ch.channelUser.begin()); it != ch.channelUser.end(); ++it)
		{
			Response::userPrefix(user, (*it).client_fd);
			Response::send_message((*it).client_fd, " " + msg.command + " " + validOption + " " + validArgument + "\r\n");
		}
	}
}

std::string getCreatedTimeUnix()
{
	std::stringstream ss;
	std::string ret;
	
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::time_t unix_timestamp = std::chrono::system_clock::to_time_t(now);
	ss << unix_timestamp;
	ss >> ret;
    return (ret);
}

std::string getCreatedTimeReadable()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::time_t unix_timestamp = std::chrono::system_clock::to_time_t(now);
    std::tm* ptm = std::localtime(&unix_timestamp);
    char buffer[32];
    // 포맷팅된 시간 문자열 생성
    std::strftime(buffer, 32, "%H:%M:%S %b %d %Y", ptm);
	std::string ret = buffer;
	return ret;
}

void EraseUserInChannel(Channel &ch, User &usr)
{
	std::list<User>::iterator it;

	for (it = ++(ch.channelUser.begin()); it != ch.channelUser.end(); ++it)
	{
		if ((*it).nick == usr.nick)
		{
			ch.channelUser.erase(it);
			break;
		}
	}
}

void EraseChannelInServer(Channel &ch, serverInfo &info)
{
	if (ch.channelUser.size() == 1)
	{
		for (std::list<Channel>::iterator it = ++(info.channelInServer.begin()); it != info.channelInServer.end(); ++it)
		{
			if ((*it).name == ch.name)
			{
				info.channelInServer.erase(it);
				break ;
			}
		}
	}
}

void EraseOPInChannel(Channel &ch, User &usr)
{
	std::list<User>::iterator it;

	for (it = ++(ch.operator_user.begin()); it != ch.operator_user.end(); ++it)
	{
		if ((*it).nick == usr.nick)
		{
			ch.operator_user.erase(it);
			break;
		}
	}
}
