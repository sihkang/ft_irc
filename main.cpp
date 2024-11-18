#include "IRCServer.hpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "IRC will have 3 argument" << std::endl;
		return 1;
	}

	try
	{
		IRCServer& server = IRCServer::getServer(argv[1], argv[2]);
		server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}