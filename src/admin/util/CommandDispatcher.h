#ifndef SMSC_ADMIN_UTIL_COMMAND_DISPATCHER
#define SMSC_ADMIN_UTIL_COMMAND_DISPATCHER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "admin/AdminException.h"
#include "admin/protocol/CommandReader.h"
#include "admin/protocol/ResponseWriter.h"
#include "admin/protocol/Command.h"
#include "admin/protocol/Response.h"
#include "core/network/Socket.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace smsc {
namespace admin {
namespace util {

using smsc::admin::AdminException;
using namespace smsc::admin::protocol;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadedTask;

template <class _CommandReader, class _ResponseWriter>
class CommandDispatcherTempl : public ThreadedTask {
public:
  CommandDispatcherTempl(Socket * admSocket, const char * const loggerCatname = "smsc.admin.util.CommandDispatcher")
    : logger(smsc::logger::Logger::getInstance(loggerCatname)),
      task_name("CommandDispatcher"),
      reader(admSocket), writer(admSocket)
  {
    sock = admSocket;
    //memcpy(cl_addr, client_addr, 15);
    //cl_addr[15] = 0;
    smsc_log_debug(logger, "Command dispatcher created.");
  }

  virtual ~CommandDispatcherTempl() {
    smsc_log_debug(logger, "Command dispatcher \"%s\" destroyed.", task_name);
    if (sock != 0) {
      sock->Abort();
      delete sock;
      sock = 0;
    }
  }

  virtual Response *handle(const Command * const command) throw (AdminException) = 0;

  virtual int Execute(){
    init();

    smsc_log_info(logger, "Command dispather started");

    std::auto_ptr<Command> command(0);
    do {
      std::auto_ptr<Response> response(0);
      try {
        while (!reader.canRead()) {
          if (isStopping)
            break;
        }
        if (!isStopping) {
          command.reset(reader.read());
          response.reset(handle(command.get()));
        }
      }
      catch (AdminException &e) {
        response.reset(new Response(Response::Error, e.what()));
        smsc_log_warn(logger, "Command dispatching failed with exception: %s", e.what());
      }
      catch (char * e) {
        response.reset(new Response(Response::Error, e));
        smsc_log_warn(logger, "Command dispatching failed with exception: %s", e);
      }
      catch (...) {
        response.reset(new Response(Response::Error, "Command dispatching failed with unknown exception"));
        smsc_log_warn(logger, "Command dispatching failed with unknown exception");
      }

      if (!isStopping) {
        // writing response
        try {
          if (response.get() == 0) {
            response.reset(new Response(Response::Error, 0));
          }
          writer.write(*response);
        }
        catch (AdminException &e) {
          smsc_log_warn(logger, "Response writing failed with exception: %s", e.what());
          break;
        }
        catch (char * e) {
          smsc_log_warn(logger, "Response writing failed with exception: %s", e);
          break;
        }
        catch (...) {
          smsc_log_warn(logger, "Response writing failed with unknown exception");
          break;
        }
      }
    } while (!isStopping && command.get() != 0
    && (command->getId() != Command::undefined));

    if (isStopping) {
      smsc_log_debug(logger, "Command dispather stopped by flag");
    }
    else {
      smsc_log_debug(logger, "Command dispather stopped by socket");
    }

    sock->Abort();
    delete sock;
    sock = 0;
    smsc_log_info(logger, "Command dispather stopped");
    return 0;
  }

  virtual const char* taskName(){
    return task_name;
  }

  static void shutdown()
  {}

protected:
  Socket * sock;
  smsc::logger::Logger *logger;
  const char * const task_name;
  char cl_addr[16];
  _CommandReader reader;
  _ResponseWriter writer;

  void init()
  {
    char thr[11];
    snprintf(thr, sizeof(thr), "[%.8X]", unsigned(pthread_self()));
    std::string ndc;
    ndc += thr;
    //ndc += cl_addr;
  }

};

typedef CommandDispatcherTempl<CommandReader, ResponseWriter> CommandDispatcher;

}
}
}
#endif // ifndef SMSC_ADMIN_UTIL_COMMAND_DISPATCHER
