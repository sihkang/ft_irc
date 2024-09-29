/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sihkang <sihkang@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 13:46:56 by sihkang           #+#    #+#             */
/*   Updated: 2024/07/02 12:00:12 by sihkang          ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>  // 입출력 스트림
#include <cstring>  // C 스타일 문자열 처리
#include <vector> // 동적 배열을 사용하기 위해 포함
#include <algorithm> // 표준 알고리즘 함수를 사용하기 위해 포함
#include <fcntl.h> // 파일 제어 옵션을 설정하기 위해 포함
#include <unistd.h> // 유닉스 표준 함수 정의를 포함
#include "../IRCServer.hpp"
#include "tools.hpp"

struct User;
struct Channel;
struct serverInfo;
struct IRCMessage;

class Response
{
public:
	static void WHOIS(int client_fd, User &user);
	static void TOPIC(int client_fd, IRCMessage message, serverInfo &info);
	static void MODE(int client_fd, IRCMessage message, serverInfo &info);
	static void INVITE(int client_fd, IRCMessage message, serverInfo &info);
	static void	KICK(int client_fd, IRCMessage message, serverInfo &info);
	static void QUIT(int client_fd, serverInfo &info);
	static void DCC(IRCMessage message, User &sender, serverInfo &info);
	static void ToChannelUser(int client_fd, IRCMessage message, serverInfo &info, bool includeMe);
	static void addNewChannel(User &requestUser, std::string chName, serverInfo &info);
	static void ChannelModeToUser(int client_fd, IRCMessage message, Channel &ch);
	static void getChannelInfo(int client_fd, User &requestUser, Channel &ch);
	static void KickInformToChannelUser(int client_fd, IRCMessage message, serverInfo &info);

	// in ServerMessage.cpp
	static void send_message(int client_fd, std::string message);
	static void requestForRegi(int client_fd);
	static void checkMessage(int client_fd, IRCMessage message, serverInfo &info);
	static void joinToChannel(int client_fd, IRCMessage message, serverInfo &info);
	static void userPrefix(User &user, int receiveSocket);


	// exception
	class sendMessageException : public std::exception
	{
	public:
		virtual const char * what() const throw()
		{
			return "[Exception] Error : Message sending failed";
		}

	};
	
	static void rpl_connection(int client_fd, User &user, serverInfo &info);
	static void rpl_passCorrect(int client_fd, serverInfo &info);	
	static void rpl421(int client_fd);
	static void rpl432(int client_fd, std::string nick);
	static void rpl441(int client_fd, User &user, IRCMessage message);
	static void rpl442(int client_fd, User &user, std::string chName);
	static void rpl461(int client_fd, User &user, IRCMessage message);
	static void rpl465(int client_fd);
	static void rpl482(int client_fd, User &user, std::string chName);
	static void rpl472(int client_fd, User &user, char wrongMode);
	static void rpl464(int client_fd);	
	static void rpl470(int client_fd, serverInfo &info, std::string chName);

	static void rpl401_modeErr(int client_fd, User &OPuser, std::string targetUser);
};

#endif