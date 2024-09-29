/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_kick.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihkang <sihkang@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 18:47:37 by sihkang           #+#    #+#             */
/*   Updated: 2024/06/28 13:12:45 by sihkang          ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "../Messages/Response.hpp"

void Response::KICK(int client_fd, IRCMessage message, serverInfo &info)
{
	User &kickingUser = findUser(info, client_fd);
	Channel &ch = findChannel(info, message.params[0].erase(0, 1));	
	User &kickedUser = findUser(ch, message.params[1]);
	
	if (message.numParams < 2)
	{
		rpl461(client_fd, kickingUser, message);
		return ;
	}

	if (findUser(ch, kickingUser.nick).nick == "")
	{
		rpl442(client_fd, kickingUser, ch.name);
		return ;
	}
	
	if (findOPUser(ch, kickingUser.nick).nick == "")
	{
		rpl482(client_fd, kickingUser, message.params[0]);
		return ;
	}
	
	if (kickedUser.nick == "")
	{
		rpl441(client_fd, kickingUser, message);
		return ;
	}
	
	Response::KickInformToChannelUser(client_fd, message, info);
	EraseOPInChannel(ch, kickedUser);
	EraseUserInChannel(ch, kickedUser);
	EraseChannelInServer(ch, info);
}

void Response::KickInformToChannelUser(int client_fd, IRCMessage message, serverInfo &info)
{
	User &sender = findUser(info, client_fd);
	Channel &receivedChannel = findChannel(info, message.params[0]);
	User &kicked = findUser(info, message.params[1]);
	std::string reason = "";
	int arguIdx = 2;
	
	while (message.numParams >= arguIdx + 1)
		reason += message.params[arguIdx++] + " ";
	
	std::list<User>::iterator it;

	for (it = ++(receivedChannel.channelUser.begin()); it != receivedChannel.channelUser.end(); ++it)
	{
		userPrefix(sender, (*it).client_fd);
		send_message((*it).client_fd, " " + message.command + " #" + receivedChannel.name + " "
					+ kicked.nick + " :" + reason + "\r\n");
	}
}
