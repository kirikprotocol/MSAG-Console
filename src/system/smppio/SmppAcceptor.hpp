#ifndef __SYSTEM_SMPPIO_SMPPACCEPTOR_HPP__
#define __SYSTEM_SMPPIO_SMPPACCEPTOR_HPP__

#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "system/smppio/SmppSocketsManager.hpp"
#include <string.h>
#include <exception>

namespace smsc{
namespace system{
namespace smppio{

class SmppAcceptorException:public std::exception{
public:
  SmppAcceptorException(const char* msg):message(msg){}
  virtual const char* what(){return message;}
protected:
  const char* message;
};

class SmppAcceptor:public smsc::core::threads::ThreadedTask{
public:
  SmppAcceptor(const char* host,int srvport,SmppSocketsManager* sockman):
    port(srvport),sm(sockman)
  {
    if(strlen(host)>sizeof(server))
    {
      throw SmppAcceptorException("too long host name");
    }
    strcpy(server,host);
  }
  virtual int Execute();
  virtual const char* taskName(){return "SmppAcceptor";}
protected:
  int port;
  char server[64];
  SmppSocketsManager* sm;
};


};//smppio
};//system
};//smsc

#endif
