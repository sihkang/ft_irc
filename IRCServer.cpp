#include "IRCServer.hpp"
#include "FDMatcher.hpp"

IRCServer& IRCServer::getServer(const char* port, const char* password)
{
	static IRCServer server(port, password);
	return server;
}

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

	this -> listen_fd = createBind(port);
	setNonblocking(this -> listen_fd);
	listen(this -> listen_fd, SOMAXCONN);

	struct pollfd pfd;
	pfd.fd = this -> listen_fd; 
	pfd.events = POLLIN;
	poll_fd.push_back(pfd);
}

IRCServer::~IRCServer() throw()
{
	close(listen_fd);
	for (size_t i = 1; i < poll_fd.size(); ++i)
		close(poll_fd[i].fd);
}

int IRCServer::createBind(const char* port)
{
	struct addrinfo want;
	struct addrinfo *result;
	struct addrinfo *rp;
	int sfd;

	std::memset(&want, 0, sizeof(struct addrinfo));
	want.ai_family = AF_UNSPEC;
	want.ai_socktype = SOCK_STREAM;
	want.ai_flags = AI_PASSIVE;
	
	int s = getaddrinfo(NULL, port, &want, &result);
	if (s != 0)
	{
		std::cerr << "getaddrinfo_error: " << gai_strerror(s) << std::endl;
		exit(1);
	}
	for (rp = result; rp != NULL; rp = rp -> ai_next)
	{
		sfd = socket(rp -> ai_family, rp -> ai_socktype, rp -> ai_protocol);
		if (sfd == -1)
			continue;
		int ok = 1;
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(int)) == -1)
		{
			std::cerr << "setsockopt" << std::endl;
			exit(1);
		}
		
		if (bind(sfd, rp -> ai_addr, rp -> ai_addrlen) == 0)
			break;
		close(sfd);
	}
	if (rp == 0)
	{
		std::cerr << "not bind" << std::endl;
		exit(1);
	}

	freeaddrinfo(result); 
	return sfd;
}

void IRCServer::setNonblocking(int cfd)
{
	int flags = fcntl(cfd, F_SETFL, O_NONBLOCK);
	if (flags == -1)
	{
		std::cerr << "fcntl error" << std::endl;
		exit(1);
	}
}

void IRCServer::run()
{
	while (true) 
	{
		int count_poll = poll(&poll_fd[0], poll_fd.size(), -1);
		if (count_poll == -1)
		{
			std::cerr << "poll error" << std::endl;
			exit(1);
		}
		for (size_t i = 0; i < poll_fd.size(); i++) 
		{
			if (poll_fd[i].revents & POLLIN)
			{
				if (poll_fd[i].fd == listen_fd) 
					connection_handling();
				else 
					messageHandling(poll_fd[i].fd);
			} 
		}
	}
}

void IRCServer::connection_handling()
{
	struct sockaddr_in client_address;
	
	socklen_t client_len; 
	int client_fd;

	client_len = sizeof(client_address);
	client_fd = accept(listen_fd, (struct sockaddr*)&client_address, &client_len);
	if (client_fd == -1)
	{
		std::cerr << "accept error" << std::endl;
		return ;
	}
	setNonblocking(client_fd);
	struct pollfd pfd;

	pfd.fd = client_fd;
	pfd.events = POLLIN;
	poll_fd.push_back(pfd);
	client_buffers[client_fd] = "";
}

void IRCServer::messageHandling(int client_fd)
{
    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE);
	int nread = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (nread <= 0)
    {
        client_remove(client_fd);
        return ;
    }
    client_buffers[client_fd] += std::string(buffer, nread);
    size_t pos;
	size_t pos_temp = temp_message[client_fd].length();
    while (1)
    {
        pos = client_buffers[client_fd].find("\r\n");
        if (pos == std::string::npos) {
			temp_message[client_fd] += client_buffers[client_fd].substr(0, pos);
			client_buffers[client_fd].clear();
            break;
		}
		pos_temp += pos;
		temp_message[client_fd] += client_buffers[client_fd].substr(0, pos);
		client_buffers[client_fd].erase(0, pos + 2);
        std::string message = temp_message[client_fd].substr(0, pos_temp);
        temp_message[client_fd].clear(); 
        IRCMessageParse(message);
        Response::checkMessage(client_fd, parsedMessage, serverinfo);
        memset(&parsedMessage, 0, sizeof(parsedMessage));
    }
}

void IRCServer::client_remove(int client_fd)
{
	close(client_fd);
	Response::QUIT(client_fd, this->serverinfo);
	poll_fd.erase(std::remove_if(poll_fd.begin(), poll_fd.end(), FDMatcher(client_fd)), poll_fd.end());
	client_buffers.erase(client_fd);
}
