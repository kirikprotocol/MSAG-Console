#include "ServiceSocketListener.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include <admin/service/ServiceCommandHandler.h>

namespace smsc {
namespace admin {
namespace service {

ServiceSocketListener::ServiceSocketListener(unsigned int _port)
	throw (AdminException&)
{
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		throw AdminException("socket fails");
	}
	port = htons(_port);
}

void ServiceSocketListener::Run() {
	sockaddr_in addr = {AF_INET, port, INADDR_ANY};
	if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		throw AdminException("bind fails");
	}

	if (listen(sock, 5))
	{
		throw AdminException("listen fails");
	}

	while (true)
	{
		sockaddr_in cl_addr = {0};
		unsigned int cl_addr_size = sizeof(cl_addr);

		int ns;
		if ((ns=accept(sock, (sockaddr*)(&cl_addr), &cl_addr_size))==-1)
		{
			throw AdminException("accept fails");
		}

		int pid = fork();
		if (pid == -1)
		{
			throw AdminException("fork fails");
		}

		if (pid == 0) // client thread
		{
			close(sock);
			ServiceCommandHandler commandHandler(ns);
			commandHandler.Run();
			exit(0);
		} else {
			close (ns);
		}
	}
}

}
}
}

