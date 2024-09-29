/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rpl.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihkang <sihkang@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 15:36:03 by sihkang           #+#    #+#             */
/*   Updated: 2024/07/02 12:02:49 by sihkang          ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

void Response::rpl_connection(int client_fd, User &user, serverInfo &info)
{
	if (user.nickComplete == true)
	{
		send_message(client_fd, ":dokang 001 " + user.nick + " :Welcome to the ft_irc Network dokang!\r\n");
		send_message(client_fd, ":dokang 002 " + user.nick + " :Your host is ft_irc by dokang\r\n");
		send_message(client_fd, ":dokang 003 " + user.nick + " :This server was created " + info.serverCreatedTime + "\r\n");
		send_message(client_fd, ":dokang 004 " + user.nick + " dokang dokangv1 io itkol :bklov\r\n");
		send_message(client_fd, ":dokang 005 " + user.nick + " CASEMAPPING=rfc1459 :are supported by this server\r\n");
	}
}

void Response::rpl_passCorrect(int client_fd, serverInfo &info)
{
	if (info.usersInServer.size() > MAXNUM_USER)
	{
		rpl465(client_fd);
		return ;
	}
	send_message(client_fd, "Password Correct! Register user infomation \"NICK <nickname>\" \"USER <username> <hostname> <servername> :<realname>\" \r\n");
	User new_user;
	new_user.client_fd = client_fd;
	new_user.auth = true;
	new_user.nickComplete = false;
	new_user.userComplete = false;
	info.usersInServer.push_back(new_user);
}

void Response::rpl465(int client_fd)
{
	send_message(client_fd, ":dokang 465 :Server is full, no new connections allowed\r\n");

}

void Response::rpl470(int client_fd, serverInfo &info, std::string chName)
{
	send_message(client_fd, ":dokang 470 " + findUser(info, client_fd).nick + " #" + chName 
				+ " :Unable to create channel, the channel limit has been reached\r\n");
}

void Response::rpl461(int client_fd, User &user, IRCMessage message)
{
	send_message(client_fd, ":dokang 461 " + user.nick + " "
				+ message.command + " :Need more parameters\r\n");
}

void Response::rpl421(int client_fd)
{
	send_message(client_fd, ":dokang 421 \r\n");
}

void Response::rpl432(int client_fd, std::string nick)
{
	send_message(client_fd, ":dokang 432 " + nick + " :Erroneous nickname\r\n");
}

void Response::rpl442(int client_fd, User &user, std::string chName)
{
	send_message(client_fd, ":dokang 442 " + user.nick + " #" + chName 
				+ " :You are not on that channel.\r\n");
}

void Response::rpl464(int client_fd)
{
	send_message(client_fd, ":localhost 464 Error: Password incorrect.\r\n");
}

void Response::rpl482(int client_fd, User &user, std::string chName)
{
	send_message(client_fd, ":dokang 482 " + user.nick + " " + chName
		+ " :You must be a channel op.\r\n");
}

void Response::rpl441(int client_fd, User &user, IRCMessage message)
{
	send_message(client_fd, ":dokang 441 " + user.nick + " " + message.params[1] + " " + message.params[0]
				+ " :They are not on that channel\r\n");
}

void Response::rpl472(int client_fd, User &user, char wrongMode)
{
	send_message(client_fd, ":dokang 472 " + user.nick + " " 
				+ wrongMode + " :is not a recognised channel mode.\r\n");
}

void Response::rpl401_modeErr(int client_fd, User &OPuser, std::string targetUser)
{
		Response::send_message(client_fd, ":dokang 401 " + OPuser.nick + " "
							+ targetUser + " :No such nick\r\n");
}