#include "IRCServer.hpp"
#include "FDMatcher.hpp"

IRCServer::IRCServer(const char *port, const char* password)
{
	this->serverinfo.serverName = "dokang";
	this->serverinfo.server_pwd = password;
	this->serverinfo.serverCreatedTime = getCreatedTimeReadable();

	User dummyUser;
	dummyUser.nick = "";
	dummyUser.auth = false;
	dummyUser.client_fd = -1;
	this->serverinfo.usersInServer.push_back(dummyUser);

	Channel dummyCh;
	dummyCh.name = "";
	dummyCh.channelUser.push_back(dummyUser);
	dummyCh.operator_user.push_back(dummyUser);
	
	this->serverinfo.channelInServer.push_back(dummyCh);

	this -> listen_fd = create_bind(port); // 주어진 포트에 바인딩된 소켓을 생성
	non_blocking(this -> listen_fd); // 소켓을 논블로킹 모드로 설정
	listen(this -> listen_fd, SOMAXCONN); // 소켓을 수신 대기 상태로 설정

	struct pollfd pfd; // 새로운 폴 파일 디스크립터 구조체를 만듭니다.
	pfd.fd = this -> listen_fd; // 폴 구조체의 파일 디스크립터를 listen 소켓으로 설정합니다.
	pfd.events = POLLIN; // 읽기 가능한 이벤트를 설정합니다.
	poll_fd.push_back(pfd); // listen 소켓을 폴링할 파일 디스크립터 목록에 추가합니다.
}

IRCServer::~IRCServer() throw()
{
	close(listen_fd); // listen 소켓을 닫습니다.
	for (size_t i = 1; i < poll_fd.size(); ++i)  // i = 0 은 listen 소켓이다.
		close(poll_fd[i].fd); // 모든 클라이언트 소켓을 닫습니다.
}

int IRCServer::create_bind(const char* port)
{
	struct addrinfo want;
	struct addrinfo *result;
	struct addrinfo *rp;
	int sfd;

	std::memset(&want, 0, sizeof(struct addrinfo)); // want 구조체 초기화
	want.ai_family = AF_UNSPEC; // IPv4 와 IPv6 둘다 허용
	want.ai_socktype = SOCK_STREAM; // TCP 소켓을 사용
	want.ai_flags = AI_PASSIVE; // 서버 소켓을 생성
	//addrinfo 정보는 readme에 설명이 있음
	int s = getaddrinfo(NULL, port, &want, &result); //포트에 대한 주소 정보를 얻음
	if (s != 0)
	{
		std::cerr << "getaddrinfo_error: " << gai_strerror(s) << std::endl;
		exit(1);
	}
	for (rp = result; rp != NULL; rp = rp -> ai_next) // 주소 정보 리스트를 순회
	{
		sfd = socket(rp -> ai_family, rp -> ai_socktype, rp -> ai_protocol); // 소켓 생성
		if (sfd == -1)
			continue;
		int ok = 1;
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(int)) == -1) // 소켓 옵션을 설정
		{
			std::cerr << "setsockopt" << std::endl;
			exit(1);
		}
		
		if (bind(sfd, rp -> ai_addr, rp -> ai_addrlen) == 0) // 소켓을 바인딩
			break;
		close(sfd);
	}
	if (rp == 0)
	{
		std::cerr << "not bind" << std::endl;
		exit(1);
	}

	freeaddrinfo(result); // 주소 정보 결과를 해제
	return sfd; // 생성된 소켓 파일 디스크립터를 반환
}

void IRCServer::non_blocking(int cfd)
{
	int flags = fcntl(cfd, F_SETFL, O_NONBLOCK); // 현재 파일 디스크립터을 논블로킹 모드로 설정
	if (flags == -1)
	{
		std::cerr << "fcntl error" << std::endl;
		exit(1);
	}
}

void IRCServer::run()
{
	while (true) // 서버가 종료될 때까지 실행
	{
		int count_poll = poll(&poll_fd[0], poll_fd.size(), -1); //폴링, 이벤트가 발생할 떄까지 대기
		if (count_poll == -1)
		{
			std::cerr << "poll error" << std::endl;
			exit(1);
		}
		for (size_t i = 0; i < poll_fd.size(); i++) // 모든 파일 디스크립터를 순회
		{
			if (poll_fd[i].revents & POLLIN) // 읽기 가능한 이벤트가 발생했는지 확인
			{
				if (poll_fd[i].fd == listen_fd) // 리스닝 소켓에서 새로운 연결이 들어왔는지 확인
				{
					connection_handling(); // 새로운 연결을 처리
				}
				else {
					message_handling(poll_fd[i].fd); // 클라이언트로부터의 메시지를 처리
				}

			} 
		}
	}
}

void IRCServer::connection_handling()
{
	struct sockaddr_in client_address; // 클라이언트 주소 정보를 저장
	// sockaddr_in 구조체에 담는 이유는 sockaddr 구조체보다 IPv4 주소를 구체적으로 담을 수 있는 구조체입니다.
	socklen_t client_len; // 주소 길이를 설정
	int client_fd; // accept 함수 반환값

	client_len = sizeof(client_address); // 주소 길이 설정
	client_fd = accept(listen_fd, (struct sockaddr*)&client_address, &client_len); // 클라이언트 연결을 수락합니다.
	if (client_fd == -1)
	{
		std::cerr << "accept error" << std::endl;
		return ;
	}
	non_blocking(client_fd); // 클라이언트 소켓을 논블로킹 모드로 설정
	struct pollfd pfd; // 새로운 폴 파일 디스크립터 구조체를 만듬

	pfd.fd = client_fd; // 폴 구조체의 파일 디스크립터를 클라이언트 소켓으로 설정
	pfd.events = POLLIN; // 읽기 가능한 이벤트를 설정
	poll_fd.push_back(pfd); // 클라이언트 소켓을 폴링할 파일 디스크립터 목록에 추가
	client_buffers[client_fd] = ""; // 버퍼 초기화
}

// TODO : 1. 각 클라이언트에 할당된 메세지 버퍼를 보관해야함
// 		2. 해당 버퍼에 메세지가 남아있으면 이전에 남아있는 메시지와 현재 들어온 메세지를 합쳐야함
//		3. 메세지를 합쳐서 보관한 것을, \r\n이 들어왔으면 한번에 처리를 해야함
void IRCServer::message_handling(int client_fd)
{
    char buffer[BUFFER_SIZE]; // 버퍼를 선언하고 초기화합니다
    std::memset(buffer, 0, BUFFER_SIZE); // 버퍼를 0으로 초기화합니다.
    // int nread = read(client_fd, buffer, BUFFER_SIZE); // 클라이언트 소켓으로부터 데이터를 읽음
	int nread = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (nread == -1)
    {
        std::cerr << "read error" << std::endl;
		Response::QUIT(client_fd, this->serverinfo); // added by sihwan
        client_remove(client_fd);
        return ;
    }
    if (nread == 0) // 읽은 데이터가 없으면 클라이언트 제거
    {
		Response::QUIT(client_fd, this->serverinfo); // added by sihwan
        client_remove(client_fd);
        return ;
    }
    client_buffers[client_fd] += std::string(buffer, nread); // 읽은 데이터를 버퍼에 추가
    size_t pos;
	size_t pos_temp = temp_message[client_fd].length();
    while (1)
    {
        pos = client_buffers[client_fd].find("\r\n"); // 버퍼에서 줄바꿈 문자를 찾음
        if (pos == std::string::npos) {
			// 기존에 들어온 메세지를 보관해서, 나중에 들어오는 메세지랑 합쳐서 처리하는 부분이 있어야함
			temp_message[client_fd] += client_buffers[client_fd].substr(0, pos);
			client_buffers[client_fd].clear();
            break;
		}
		pos_temp += pos;
		temp_message[client_fd] += client_buffers[client_fd].substr(0, pos);
		client_buffers[client_fd].erase(0, pos + 2);
        std::string message = temp_message[client_fd].substr(0, pos_temp); //메시지 추출
        temp_message[client_fd].clear(); // 추출한 메시지를 버퍼에서 제거
        this->IRCMessageParse(message);
        Response::checkMessage(client_fd, parsedMessage, serverinfo);
        memset(&parsedMessage, 0, sizeof(parsedMessage)); // 파싱된 메시지를 담는 구조체 초기화
    }
}

void IRCServer::client_remove(int client_fd)
{
	close(client_fd); // 클라이언트 소켓을 닫음
	poll_fd.erase(std::remove_if(poll_fd.begin(), poll_fd.end(), FDMatcher(client_fd)), poll_fd.end());
	// 벡터에서 FDMatcher 함수 객체를 사용하여 클라이언트 파일 디스크립터와 일치하는 요소를 찾아 제거합니다.
	client_buffers.erase(client_fd); // 맵에서 클라이언트 제거
}
