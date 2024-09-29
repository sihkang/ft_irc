/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_mode.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihwan <sihwan@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 14:13:57 by sihkang           #+#    #+#             */
/*   Updated: 2024/06/30 12:07:39 by sihwan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Messages/Response.hpp"

void Response::MODE(int client_fd, IRCMessage message, serverInfo &info)
{
	User &usr = findUser(info, client_fd);
	if (message.params[0].front() != '#')
	{
		return ;
	}

	Channel &ch = findChannel(info, message.params[0].erase(0, 1));
	if (message.numParams == 1)
	{
		Response::getChannelInfo(client_fd, usr, ch);
	}
	else
	{
		if (message.params[1] == "b")
			return ;
		if (findOPUser(ch, client_fd).nick == "")
		{
			send_message(client_fd, ":dokang 482 " + usr.nick + " #" 
						+ ch.name + " :You must be a channel op\r\n");
			return ;
		}

		changeChannelMode(client_fd, ch, message);
	}
	send_message(client_fd, "\r\n");
}

void Response::ChannelModeToUser(int client_fd, IRCMessage message, Channel &ch)
{
	User &sender = findUser(ch, client_fd);
	
	std::list<User>::iterator it;
	std::string params = '#' + message.params[0];
	int i;
	for (i = 1; i < message.numParams - 1; i++)
	{
		params += " " + message.params[i];
	}
	params += " :" + message.params[i];

	for (it = ++(ch.channelUser.begin()); it != ch.channelUser.end(); ++it)
	{
		userPrefix(sender, (*it).client_fd);
		send_message((*it).client_fd, " " + message.command + " " + params + "\r\n");
	}
}

void Response::getChannelInfo(int client_fd, User &requestUser, Channel &ch)
{
	send_message(client_fd, ":dokang 324 " + requestUser.nick + " #" 
				+ ch.name + " :" + getChannelMode(ch) + "\n");
	send_message(client_fd, ":dokang 329 " + requestUser.nick + " #"
				+ ch.name + " :" + ch.createdTime + "\r\n");	
}
