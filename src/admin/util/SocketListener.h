#ifndef SMSC_ADMIN_UTIL_SOCKET_LISTENER
#define SMSC_ADMIN_UTIL_SOCKET_LISTENER

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <log4cpp/Category.hh>
#include <admin/AdminException.h>
#include <core/network/Socket.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/threads/Thread.hpp>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace util {

using log4cpp::Category;
using smsc::admin::AdminException;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::Thread;
using smsc::util::Logger;

template<class _T_CommandDispatcher>
class SocketListener : public Thread
{
public:

	SocketListener(const char * const debugCategory = "smsc.admin.util.SocketListener")
		throw (AdminException)
		: logger(Logger::getCategory(debugCategory))
	{
	}
	
	void init(const char * const hostName, in_port_t portToListen)
		throw (AdminException)
	{
		isShutdownSignaled = false;

		if (sock.InitServer(hostName, portToListen, 10) != 0)
		{
			throw AdminException("socket fails");
		}
		logger.info("socket listener ready to start on port %i", portToListen);
	}

	virtual int Execute()
	{
		//ShutdownableList::addListener(this);
		sock.StartServer();
		
		logger.info("socket listener started");
		
		while (!isShutdownSignaled)
		{
			if (Socket *newSocket = sock.Accept())
			{
				pool.startTask(new _T_CommandDispatcher(newSocket));
			}
			else
			{
				if (isShutdownSignaled)
					logger.info("ServiceSocketListener shutdown");
			}
		}
		
    sock.Abort();
		logger.info("ServiceSocketListener stopped");
		return 0;
	}

	void shutdown()
	{
		pool.shutdown();
    sock.Close();
    logger.debug("ServiceSocketListener: server socket closed");
		isShutdownSignaled = true;
	}

protected:
	Socket sock;
	bool isShutdownSignaled;
	ThreadPool pool;
	Category &logger;
};

}
}
}

#endif
