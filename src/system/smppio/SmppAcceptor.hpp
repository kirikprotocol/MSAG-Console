#ifndef __SYSTEM_SMPPIO_SMPPACCEPTOR_HPP__
#define __SYSTEM_SMPPIO_SMPPACCEPTOR_HPP__

#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/Event.hpp"
#include "system/smppio/SmppSocketsManager.hpp"
#include <string.h>
#include "util/Exception.hpp"
#include "util/debug.h"


namespace smsc{
namespace system{
namespace smppio{

using smsc::util::Exception;
using smsc::core::synchronization::Event;

class SmppAcceptor:public smsc::core::threads::ThreadedTask{
public:
  SmppAcceptor(const char* host,int srvport,SmppSocketsManager* sockman,Event* sn):
    port(srvport),sm(sockman),started(false),startNotify(sn)
  {

    if(strlen(host)>sizeof(server))
    {
      throw Exception("too long host name");
    }
    strcpy(server,host);
  }
  virtual int Execute();
  virtual const char* taskName(){return "SmppAcceptor";}
  bool isStarted(){return started;}
protected:
  int port;
  char server[64];
  SmppSocketsManager* sm;
  bool started;
  Event *startNotify;
};


};//smppio
};//system
};//smsc

#endif
