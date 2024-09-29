#ifndef FDMATCHER_HPP
#define FDMATCHER_HPP

#include <poll.h>

class FDMatcher {
	public:
		FDMatcher(int fd);
		bool operator()(const struct pollfd& pfd) const;
	private:
		int client_fd;
};
#endif