/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMessage.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihkang <sihkang@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 14:11:25 by sihkang           #+#    #+#             */
/*   Updated: 2024/11/18 11:11:35 by sihkang          ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

void Response::checkMessage(int client_fd, IRCMessage message, serverInfo &info)
{
	if (isCommand(message, "JOIN"))
	{
		if (!numParamCheck(client_fd, message, 1))
			return ;
		if (message.params[0] == ":")
			Response::requestForRegi(client_fd);
		else
			Response::joinToChannel(client_fd, message, info);
	}
	else if (isCommand(message, "PASS"))
	{
		if (message.numParams == 1 && isCorrectPassword(info, message.params[0]))
		{
			rpl_passCorrect(client_fd, info);
		}
		else
		{
			rpl464(client_fd);
		}
	}
	else if (isCommand(message, "NICK"))
	{
		if (!numParamCheck(client_fd, message, 1))
			return ;
		User &user = findUser(info, client_fd);
		if (user.client_fd > 2 && user.auth == true)
		{
			if (findUser(info, message.params[0]).nick != "")
				send_message(client_fd, ":dokang 433 * " + message.params[0] + " :Nickname is already in use.\r\n");
			else if (!isValidNick(message.params[0]))
			{
				rpl432(client_fd, message.params[0]);
			}
			else
			{
				user.nick = message.params[0];
				user.nickComplete = true;
				if (user.nickComplete && user.userComplete)
					rpl_connection(client_fd, user, info);
			}
		}
	}
	else if (isCommand(message, "USER"))
	{
		if (!numParamCheck(client_fd, message, 4))
			return ;
		User &user = findUser(info, client_fd);
		if (user.auth == true)
		{
			user.username = message.params[0];
			user.hostname = message.params[1];
			user.servername = message.params[2];
			user.realname = aftercolonConcat(message);
			user.userComplete = true;
			if (user.nickComplete && user.userComplete)
				rpl_connection(client_fd, user, info);
		}
	}
	else if (isCommand(message, "PRIVMSG"))
	{
		if (!numParamCheck(client_fd, message, 2))
			return ;
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			Response::ToChannelUser(client_fd, message, info, false);			
		}
	}
	else if (isCommand(message, "KICK"))
	{
		if (!numParamCheck(client_fd, message, 1))
			return ;
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			Response::KICK(client_fd, message, info);		
		}
	}
	else if (isCommand(message, "INVITE"))
	{
		if (!numParamCheck(client_fd, message, 1))
			return ;
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			Response::INVITE(client_fd, message, info);
		}
	}
	else if (isCommand(message, "TOPIC"))
	{
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			Response::TOPIC(client_fd, message, info);
		}
	}
	else if (isCommand(message, "MODE"))
	{
		if (!numParamCheck(client_fd, message, 1))
			return ;
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			Response::MODE(client_fd, message, info);	
		}
	}
	else if (isCommand(message, "PING"))
	{
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			send_message(client_fd, "PONG ft_irc local\r\n");
		}
	}
	else if (isCommand(message, "QUIT"))
	{
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		else if (user.auth && user.nickComplete && user.userComplete)
		{
			Response::QUIT(client_fd, info);
		}
	}
	else
	{
		User &user = findUser(info, client_fd);
		if (user.nick == "")
			return ;
		Response::rpl421(client_fd);
	}
}

