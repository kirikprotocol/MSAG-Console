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
#include <util/debug.h>
#include <core/synchronization/EventMonitor.hpp>

namespace smsc {
namespace admin {
namespace util {

using log4cpp::Category;
using smsc::admin::AdminException;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::Thread;
using smsc::util::Logger;
using smsc::core::synchronization::EventMonitor;

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
//      sock.Abort();
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
