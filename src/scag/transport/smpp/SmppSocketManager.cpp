#include "SmppSocketManager.h"

namespace scag{
namespace transport{
namespace smpp{

void SmppSocketManager::registerSocket(SmppSocket* sock)
{
  sock->setSocketManager(this);
  sock->setInterfaces(queue,reg);
  for(int i=0;i<readers.Count();i++)
  {
    if(readers[i]->getSocketsCount()<MaxSocketsPerThread)
    {
      info2(log,"Reusing reader/writer (%d)",readers[i]->getSocketsCount());
      readers[i]->addSocket(sock);
      writers[i]->addSocket(sock);
      sock->release();
      return;
    }
  }
  info2(log,"Creating new reader/writer (%d)",readers.Count());
  SmppReader* rd=new SmppReader();
  SmppWriter* wr=new SmppWriter();
  readers.Push(rd);
  writers.Push(wr);
  rd->addSocket(sock);
  wr->addSocket(sock);
  sock->release();
  tp.startTask(rd);
  tp.startTask(wr);
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
  conn->shutdown();
  tp.shutdown();
  readers.Empty();
  writers.Empty();
}


}//smpp
}//transport
}//scag
