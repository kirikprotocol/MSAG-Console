#include "system/smppio/SmppAcceptor.hpp"
#include <string.h>
#include <errno.h>

namespace smsc{
namespace system{
namespace smppio{

using namespace smsc::core::network;

int SmppAcceptor::Execute()
{
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
  startNotify->Signal();

  linger l;
  l.l_onoff=1;
  l.l_linger=0;
  setsockopt(srv.getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));

  for(;;)
  {
    clnt=srv.Accept();
    if(!clnt)
    {
      trace2("accept failed. error:%s",strerror(errno));
      //break;
      if(isStopping)break;
      continue;
    }
    trace2("Connection accepted:%p",clnt);
    sm->registerSocket(clnt);
  }
  trace("SmppAcceptor: quiting");
  return 0;
}

};//smppio
};//system
};//smsc
