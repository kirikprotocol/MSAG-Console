#include "system/smppio/SmppAcceptor.hpp"

namespace smsc{
namespace system{
namespace smppio{

using namespace smsc::core::network;

int SmppAcceptor::Execute()
{
  Socket srv;
  if(srv.InitServer(server,port,0)==-1)
    throw SmppAcceptorException("Failed to init smpp server socket");
  if(srv.StartServer()==-1)
    throw SmppAcceptorException("Failed to start smpp server socket");
  Socket *clnt;
  for(;;)
  {
    clnt=srv.Accept();
    if(!clnt)break;
    sm->registerSocket(clnt);
  }
}

};//smppio
};//system
};//smsc
