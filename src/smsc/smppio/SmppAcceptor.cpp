#include <string.h>
#include <errno.h>

#include "logger/Logger.h"

#include "SmppAcceptor.hpp"

namespace smsc{
namespace smppio{

using namespace smsc::core::network;

int SmppAcceptor::Execute()
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smpp.acc");
  try{
    if(srv.InitServer(server,port,0,1,true)==-1)
      throw Exception("Failed to init smpp server socket");
    if(srv.StartServer()==-1)
      throw Exception("Failed to start smpp server socket");
  }catch(...)
  {
    startNotify->Signal();
    throw;
  }
  Socket *clnt;
  started=true;
  debug2(log,"signal smpp acceptor start:%p",startNotify);
  startNotify->SignalAll();


  while(!isStopping)
  {
    clnt=srv.Accept();
    if(isStopping)
    {
      break;
    }
    if(!clnt)
    {
      __warning2__("accept failed. error:%s",strerror(errno));
      //break;
      continue;
    }
    clnt->SetNoDelay(1);
    char buf[32];
    clnt->GetPeer(buf);
    info2(log,"Connection accepted:%p/%s",clnt,buf);
    sm->registerSocket(clnt);
  }
  srv.Close();

  __trace__("SmppAcceptor: quiting");
  return 0;
}

}//smppio
}//smsc
