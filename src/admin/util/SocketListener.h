#ifndef SMSC_ADMIN_UTIL_SOCKET_LISTENER
#define SMSC_ADMIN_UTIL_SOCKET_LISTENER

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <log4cpp/Category.hh>
#include <admin/AdminException.h>
#include <admin/util/Shutdownable.h>
#include <core/threads/ThreadPool.hpp>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace util {

using log4cpp::Category;
using smsc::admin::AdminException;
using smsc::core::threads::ThreadPool;
using smsc::util::Logger;

template<class _T_CommandDispatcher, class _T_CommandHandler>
class SocketListener : public Shutdownable {
public:

	SocketListener(in_port_t portToListen,
								 _T_CommandHandler * commandHandler,
								 const char * const debugCategory = "smsc.admin.util.SocketListener")
		throw (AdminException &)
		: logger(Logger::getCategory(debugCategory))
	{
		port = portToListen;
		sock = 0;
		isShutdownSignaled = false;
		handler = commandHandler;

		sock = socket(PF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
			throw AdminException("socket fails");
		}
	}

	void run() throw (AdminException &)
	{
		sockaddr_in addr = {AF_INET, htons(port), INADDR_ANY};
		if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == -1)
		{
			throw AdminException("bind fails");
		}

		if (listen(sock, 5))
		{
			throw AdminException("listen fails");
		}

		logger.info("socket listener started on port %i", port);

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
				pool.startTask(new _T_CommandDispatcher(this, ns, inet_ntoa(cl_addr.sin_addr), handler));
			}
		}
	}

	void shutdown()
	{
		isShutdownSignaled = true;
		if (sock != 0)
			close(sock);
		sock = 0;
	}

protected:
	in_port_t port;
	int sock;
	bool isShutdownSignaled;
	ThreadPool pool;
	Category &logger;
	_T_CommandHandler *handler;
};

}
}
}

#endif
