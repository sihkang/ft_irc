/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_topic.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihkang <sihkang@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 16:03:41 by sihkang           #+#    #+#             */
/*   Updated: 2024/06/28 13:12:18 by sihkang          ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "../Messages/Response.hpp"

void Response::TOPIC(int client_fd, IRCMessage message, serverInfo &info)
{
	if (message.numParams == 0)
	{
		rpl461(client_fd, findUser(info, client_fd), message);
		return ;
	}
	
	std::string chName = message.params[0].erase(0, 1);
	Channel &ch = findChannel(info, chName);
	User &user = findUser(ch, client_fd);
	
	std::string chTopic = aftercolonConcat(message);

	if (ch.opt[MODE_t] == true && findOPUser(ch, client_fd).nick == "")
	{
		send_message(client_fd, ":dokang 482 " + user.nick + " #" + ch.name + " :You must be a channel op\r\n");
		return ;
	}
	ch.topic = chTopic;
	Response::ToChannelUser(client_fd, message, info, true);
}
