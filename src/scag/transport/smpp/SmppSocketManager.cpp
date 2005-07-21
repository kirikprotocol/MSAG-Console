#include "SmppSocketManager.h"

namespace scag{
namespace transport{
namespace smpp{

void SmppSocketManager::registerSocket(SmppSocket* sock)
{
  sock->setSocketManager(this);
}

void SmppSocketManager::unregisterSocket(SmppSocket* sock)
{
  if(sock->getType()==etSmsc)
  {
    conn->reportSmscDisconnect(sock->getSystemId());
  }
}

void SmppSocketManager::shutdown()
{
  tp.shutdown();
  readers.Empty();
  writers.Empty();
}


}//smpp
}//transport
}//scag
