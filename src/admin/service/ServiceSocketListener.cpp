#include "ServiceSocketListener.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include <admin/service/CommandDispatcher.h>
#include <admin/service/ServiceCommandHandler.h>

namespace smsc {
namespace admin {
namespace service {

ServiceSocketListener::ServiceSocketListener(unsigned int _port,
																						 ServiceCommandHandler * commandHandler)
	throw (AdminException&)
	: logger(smsc::util::Logger::getCategory("smsc.admin.service.ServiceSocketListener"))
{
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		throw AdminException("socket fails");
	}
	port = _port;
	handler = commandHandler;
	isShutdownSignaled = false;
}

void ServiceSocketListener::run() {
	sockaddr_in addr = {AF_INET, htons(port), INADDR_ANY};
	if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		throw AdminException("bind fails");
	}

	if (listen(sock, 5))
	{
		throw AdminException("listen fails");
	}

	logger.info("Admin socket listener started on port %i", port);

	while (!isShutdownSignaled)
	{
		sockaddr_in cl_addr = {0};
		unsigned int cl_addr_size = sizeof(cl_addr);

		int ns;
		if ((ns=accept(sock, (sockaddr*)(&cl_addr), &cl_addr_size))==-1)
		{
			if (isShutdownSignaled)
				logger.info("ServiceSocketListener shutdown");
			else
				logger.warn("Socket accept fails");
		}
		else
		{
			pool.startTask(new CommandDispatcher(this, ns, inet_ntoa(cl_addr.sin_addr), handler));
		}
	}
}

void ServiceSocketListener::shutdown(void)
{
	isShutdownSignaled = true;
	if (sock != 0)
		close(sock);
	sock = 0;
}

}
}
}

