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

  virtual ~SocketListener()
  {
    abort();
  }

  void init(const char * const hostName, unsigned portToListen)
    throw (AdminException)
  {
    isShutdownSignaled = false;
    isAbortSignaled = false;

    if (sock.InitServer(hostName, portToListen, 10, 0) != 0)
    {
      throw AdminException("SocketListener: Failed to init server at %s:%d",hostName,portToListen);
    }
    smsc_log_info(logger, "socket listener ready to start on port %i", portToListen);
  }

  virtual int Execute()
  {
    try
    {
      sock.StartServer();

      smsc_log_info(logger, "socket listener started");

      while (!isShutdownSignaled)
      {
        if (Socket *newSocket = sock.Accept())
        {
          pool.startTask(new _T_CommandDispatcher(newSocket));
        }
        else
        {
          if (isShutdownSignaled) {
            smsc_log_info(logger, "ServiceSocketListener shutdown");
          }
        }
      }

      __trace2__( "ServiceSocketListener::logger 0x%p", &logger );
      smsc_log_info(logger, "ServiceSocketListener sock abort");

      if (!isAbortSignaled)
        _T_CommandDispatcher::shutdown();

      __trace2__( "ServiceSocketListener::logger 0x%p", &logger );
      smsc_log_info(logger, "ServiceSocketListener pool shutdown");
      pool.shutdown();
      __trace2__( "ServiceSocketListener::logger 0x%p", &logger );
      smsc_log_info(logger, "ServiceSocketListener stopped");

    }
    catch (std::exception &e)
    {
      smsc_log_error(logger, "Exception on listener thread: %s\n", e.what());
    }
    catch (...)
    {
      smsc_log_error(logger, "Unknown Exception on listener thread\n");
    }
    smsc_log_error(logger, "ServiceSocketListener: stopped all");
    return 0;
  }

  void shutdown()
  {
    isShutdownSignaled = true;
    sock.Abort();
    smsc_log_debug(logger, "ServiceSocketListener: shutdown: server socket closed");
    smsc_log_debug(logger, "ServiceSocketListener: dead");
  }

  void abort()
  {
    isShutdownSignaled = true;
    isAbortSignaled = true;
    sock.Abort();
    //smsc_log_debug(logger, "ServiceSocketListener: abort: server socket closed");
    //smsc_log_debug(logger, "ServiceSocketListener: dead");
  }

protected:
  Socket sock;
  bool isShutdownSignaled;
  bool isAbortSignaled;
  ThreadPool pool;
  Logger *logger;
};

}
}
}

#endif
