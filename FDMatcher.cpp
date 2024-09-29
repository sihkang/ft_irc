#include "FDMatcher.hpp"

FDMatcher::FDMatcher(int fd) : client_fd(fd) {}

bool FDMatcher::operator()(const struct pollfd& pfd) const {
	return pfd.fd == client_fd;
}