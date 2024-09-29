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

#include "Messages/Response.hpp"

#define BUFFER_SIZE 512 // 버퍼 크기를 512 바이트로 정의

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
	// 채널모드 변수 필요
	
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
	std::string server_pwd; // 서버 연결 비밀번호
	std::string serverCreatedTime;

	std::list<User> usersInServer; // 서버에 등록된 유저
	std::list<Channel> channelInServer; // 서버에 존재하는 채널

};

class IRCServer : public std::exception
{
	public:
		IRCServer(const char* port, const char* password); //서버 초기화(생성자)
		virtual ~IRCServer() throw(); //소멸자
		void run(); //서버의 메인 루프를 실행
	private:
		serverInfo serverinfo;
		std::string serverName;
		int create_bind(const char* port); // 소캣을 생성 및 포트에 바인딩
		void non_blocking(int cfd); // 소켓을 논블로킹 모드로 설정
		void connection_handling(); //새 클라이언트 연결을 처리
		void message_handling(int client_fd); // 클라이언트로부터의 메시지를 처리
		void client_remove(int client_fd); // 클라이언트를 제거
		int listen_fd; // listen 소켓 파일 디스크립터
		std::string server_pwd; // 서버 연결 비밀번호
		std::vector<struct pollfd> poll_fd; // 폴링할 파일 디스크립터 목록 

		//struct pollfd는 구조체는 readme에 설명이 있음
		std::map<int, std::string> client_buffers;
		std::map<int, std::string> temp_message; // 클라이언트 별로 수신된 데이터 버퍼를 저장

		// in IRCMessageParse.cpp
		void IRCMessageParse(std::string message);
		IRCMessage parsedMessage;
};

#endif