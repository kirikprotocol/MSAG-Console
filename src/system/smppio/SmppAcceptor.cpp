#include "system/smppio/SmppAcceptor.hpp"
#include <string.h>
#include <errno.h>
#include "logger/Logger.h"

namespace smsc{
namespace system{
namespace smppio{

using namespace smsc::core::network;

int SmppAcceptor::Execute()
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smpp.acc");
  Socket srv;
  try{
    if(srv.InitServer(server,port,0)==-1)
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


  linger l;
  l.l_onoff=1;
  l.l_linger=0;
  setsockopt(srv.getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));

  for(;;)
  {
    clnt=srv.Accept();
    if(!clnt)
    {
      __warning2__("accept failed. error:%s",strerror(errno));
      //break;
      if(isStopping)break;
      continue;
    }
    clnt->SetNoDelay(1);
    char buf[32];
    clnt->GetPeer(buf);
    info2(log,"Connection accepted:%p/%s",clnt,buf);
    sm->registerSocket(clnt);
  }
  trace("SmppAcceptor: quiting");
  return 0;
}

}//smppio
}//system
}//smsc
