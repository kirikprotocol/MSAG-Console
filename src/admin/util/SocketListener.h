#ifndef SMSC_ADMIN_UTIL_SOCKET_LISTENER
#define SMSC_ADMIN_UTIL_SOCKET_LISTENER

#include <admin/AdminException.h>
#include <core/network/Socket.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/threads/Thread.hpp>
#include <logger/Logger.h>
#include <util/debug.h>
#include <core/synchronization/EventMonitor.hpp>

namespace smsc {
namespace admin {
namespace util {

using smsc::logger::Logger;
using smsc::admin::AdminException;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::Thread;
using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;

template<class _T_CommandDispatcher>
class SocketListener : public Thread
{
public:

	SocketListener(const char * const loggerCatname = "smsc.admin.util.SocketListener")
		throw (AdminException)
		: logger(Logger::getInstance(loggerCatname))
	{
	}

	void init(const char * const hostName, unsigned portToListen)
		throw (AdminException)
	{
		isShutdownSignaled = false;
		isAbortSignaled = false;

		if (sock.InitServer(hostName, portToListen, 10, 0) != 0)
		{
			throw AdminException("socket fails");
		}
		logger.info("socket listener ready to start on port %i", portToListen);
	}

	virtual int Execute()
	{
		try 
		{
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

			__trace2__( "ServiceSocketListener::logger 0x%p", &logger );
			logger.info("ServiceSocketListener sock abort");

			if (!isAbortSignaled)
				_T_CommandDispatcher::shutdown();

			__trace2__( "ServiceSocketListener::logger 0x%p", &logger );
			logger.info("ServiceSocketListener pool shutdown");
			pool.shutdown();
			__trace2__( "ServiceSocketListener::logger 0x%p", &logger );
			logger.info("ServiceSocketListener stopped");

		}
		catch (std::exception &e)
		{
			logger.error("Exception on listener thread: %s\n", e.what());
		}
		catch (...)
		{
			logger.error("Unknown Exception on listener thread\n");
		}
		logger.error("ServiceSocketListener: stopped all");
		return 0;
	}

	void shutdown()
	{
		isShutdownSignaled = true;
		sock.Abort();
		logger.debug("ServiceSocketListener: server socket closed");
		logger.debug("ServiceSocketListener: dead");
	}

	void abort()
	{
		isShutdownSignaled = true;
		isAbortSignaled = true;
		sock.Abort();
		logger.debug("ServiceSocketListener: server socket closed");
		logger.debug("ServiceSocketListener: dead");
	}

protected:
	Socket sock;
	bool isShutdownSignaled;
	bool isAbortSignaled;
	ThreadPool pool;
	Logger logger;
};

}
}
}

#endif
