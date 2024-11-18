#ifndef IRCSERVER_HPP
# define IRCSERVER_HPP

#include <iostream>  // 입출력 스트림
#include <cstring>  // C 스타일 문자열 처리
#include <vector> // 동적 배열을 사용하기 위해 포함
#include <list>
#include <map> // 키-값 쌍을 저장할 수 있는 맵 컨테이너 사용
#include <algorithm> // 표준 알고리즘 함수를 사용하기 위해 포함
#include <sys/types.h> // 시스템 타입 정의를 포함
#include <sys/socket.h> // 소켓 프로그래밍을 위한 정의를 포함
#include <netdb.h> // 네트워크 데이터베이스 작업을 위한 정의를 포함
#include <fcntl.h> // 파일 제어 옵션을 설정하기 위해 포함
#include <unistd.h> // 유닉스 표준 함수 정의를 포함
#include <poll.h> // I/O 다중화를 위해 "poll"함수를 사용하기 위해 포함
#include <signal.h>
#include <exception>
#include <sstream>
#include <memory>

#include "Messages/Response.hpp"

#define BUFFER_SIZE 512

# define MODE_i 0
# define MODE_t 1
# define MODE_k 2
# define MODE_o 3
# define MODE_l 4

# define MAXNUM_USER 100
# define MAXNUM_CH   30

struct User;

struct Channel
{
	std::string name;
	std::string topic;
	std::string key;
	std::list<User> operator_user;
	int user_limit;
	bool opt[5]; // itkol
	std::string createdTime;
	std::list<struct User> channelUser;
} ;

struct User
{
	int client_fd;
	std::string nick;
	
	std::string username;
	std::string hostname;
	std::string servername;
	std::string realname;

	bool auth;
	bool nickComplete;
	bool userComplete;

	std::map<int, std::string> client_buffers;
};

struct IRCMessage
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	int numParams;
};

struct serverInfo
{
	std::string serverName;
	std::string server_pwd;
	std::string serverCreatedTime;
	std::list<User> usersInServer; 
	std::list<Channel> channelInServer; 

};

class IRCServer : public std::exception
{
	public:
		static IRCServer& getServer(const char* port, const char* password);
		~IRCServer() throw();
		void run();

	private:
		IRCServer();
		IRCServer(const char* port, const char* password);

		serverInfo serverinfo;

		int createBind(const char* port);
		void setNonblocking(int cfd);
		void connection_handling();
		void messageHandling(int client_fd);
		void client_remove(int client_fd);

		int listen_fd;
		std::string server_pwd;
		std::vector<struct pollfd> poll_fd;

		std::map<int, std::string> client_buffers;
		std::map<int, std::string> temp_message;

		// in IRCMessageParse.cpp
		void IRCMessageParse(std::string message);
		IRCMessage parsedMessage;

		IRCServer(const IRCServer&) = delete;
		IRCServer& operator=(const IRCServer&) = delete;

};

#endif