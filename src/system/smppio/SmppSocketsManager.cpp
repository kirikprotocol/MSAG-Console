#include "system/smppio/SmppSocketsManager.hpp"
#include "system/smppio/SmppIOTask.hpp"

namespace smsc{
namespace system{
namespace smppio{

void SmppSocketsManager::registerSocket(Socket* sock)
{
  MutexGuard g(m);
  int i;
  sock->setData(0,0);
  sock->setData(1,this);
  for(i=0;i<intasks.Count();i++)
  {
    if(intasks[i]->socketsCount()<SM_SOCK_PER_THREAD)
    {
      sock->setData(0,(void*)2);
      intasks[i]->addSocket(sock);
      outtasks[i]->addSocket(sock);
      intasks[i]->notify();
      outtasks[i]->notify();

      return;
    }
  }
  SmppOutputThread *out=new SmppOutputThread;
  SmppInputThread *in=new SmppInputThread(smeManager);

  out->assignIn(in);
  in->assignOut(out);

  sock->setData(0,(void*)2);

  in->addSocket(sock);
  out->addSocket(sock);

  intasks.Push(in);
  outtasks.Push(out);

  tp->startTask(out);
  tp->startTask(in);

}

void SmppSocketsManager::removeSocket(Socket* sock)
{
  MutexGuard g(m);
  int x=(int)sock->getData(0);
  x--;
  trace2("socket ref count for %p:%d",sock,x);
  sock->setData(0,(void*)x);
  if(sock->getData(0))return;
  trace2("deleting socket:%p",sock);
  delete sock;
}

};//smppio
};//system
};//smsc
