#include "system/smppio/SmppAcceptor.hpp"

namespace smsc{
namespace system{
namespace smppio{

using namespace smsc::core::network;

int SmppAcceptor::Execute()
{
  Socket srv;
  trace2("wtf?");
  if(srv.InitServer(server,port,0)==-1)
    throw Exception("Failed to init smpp server socket");
  if(srv.StartServer()==-1)
    throw Exception("Failed to start smpp server socket");
  Socket *clnt;
  for(;;)
  {
    clnt=srv.Accept();
    if(!clnt)break;
    trace("Connection accepted");
    sm->registerSocket(clnt);
  }
  return 0;
}

};//smppio
};//system
};//smsc
