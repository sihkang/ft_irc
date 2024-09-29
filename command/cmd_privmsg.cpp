/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_privmsg.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihkang <sihkang@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:18:57 by sihkang           #+#    #+#             */
/*   Updated: 2024/06/28 15:41:49 by sihkang          ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "../Messages/Response.hpp"

void Response::ToChannelUser(int client_fd, IRCMessage message, serverInfo &info, bool includeMe)
{
	User &sender = findUser(info, client_fd);
	std::string chName;
	if (message.params[0].front() == '#')
		chName = message.params[0].erase(0,1);
	else
		chName = message.params[0];

	Channel& receivedChannel = findChannel(info, chName);
	if (receivedChannel.name == "" && findUser(info, chName).nick != "")
		Response::DCC(message, sender, info);
	else if (receivedChannel.name == "")
		return ;


	std::list<User>::iterator it;
	for (it = ++(receivedChannel.channelUser.begin()); it != receivedChannel.channelUser.end(); ++it)
	{
		if (client_fd == (*it).client_fd && includeMe == false)
			continue;
		userPrefix(sender, (*it).client_fd);
		send_message((*it).client_fd, " " + message.command + " #" + receivedChannel.name
					+ " :" + aftercolonConcat(message) + "\r\n");
	}
}

void Response::DCC(IRCMessage message, User &sender, serverInfo &info)
{
	User &receiver = findUser(info, message.params[0]);

	Response::userPrefix(sender, receiver.client_fd);
	Response::send_message(receiver.client_fd, " " + message.command + " " + getMessageParams(message) + "\r\n");
}