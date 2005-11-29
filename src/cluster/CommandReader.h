#ifndef CommandReader_dot_h
#define CommandReader_dot_h

#include "Commands.h"
#include "core/threads/Thread.hpp"
#include "CommandDispatcher.h"
#include "Interconnect.h"
#include "core/network/Socket.hpp"

namespace smsc {
namespace cluster {

using smsc::core::threads::Thread;
using smsc::core::network::Socket;

class CommandReader : public Thread
{
public:
    CommandReader()
        : logger(smsc::logger::Logger::getInstance("CmdReader")),
          stop(true),
          role(0),
          sock(0),
          dispatcher(0),
          attachedSocket(0)
    {};
    /*
  CommandReader(Role *role_, Socket *sock_, CommandDispatcher *dispatcher_, Socket *attachedSocket_)
    : logger(smsc::logger::Logger::getInstance("CmdReader")),
          stop(true),
      role(role_),
      sock(sock_),
          dispatcher(dispatcher_),
          attachedSocket(attachedSocket_)
    {};
    */

    void Init(Role *role_, Socket *sock_, CommandDispatcher *dispatcher_, Socket *attachedSocket_)
    {
      role=role_;
      sock=sock_;
      dispatcher=dispatcher_;
      attachedSocket=attachedSocket_;
    }

    ~CommandReader()
    {
        WaitFor();
    };
  void Start();
  void Stop()
    {
        stop = true;
        WaitFor();
    };
protected:
  CommandReader(const CommandReader&);
  virtual int Execute();
    void readHeader(Socket *socket, uint32_t &type, uint32_t &len);
    Command* readCommand(Socket *socket);
    void read(Socket *socket, void * buffer, int size);
    void writeRole(Socket *socket, Command *cmd);
    smsc::logger::Logger    *logger;
  bool stop;
  Role *role;
  Socket *sock;
    CommandDispatcher * dispatcher;
    Socket *attachedSocket;
};

}
}

#endif
