#include "system/smppio/SmppSocketsManager.hpp"
#include "system/smppio/SmppIOTask.hpp"

namespace smsc{
namespace system{
namespace smppio{

void SmppSocketsManager::registerSocket(Socket* sock)
{
  MutexGuard g(mtxAdd);
  int i;
  sock->setData(0,0);
  sock->setData(1,this);
  for(i=0;i<intasks.Count();i++)
  {
    if(intasks[i]->socketsCount()<SM_SOCK_PER_THREAD)
    {
      sock->setData(0,(void*)2);
      outtasks[i]->addSocket(sock,socketTimeOut);
      intasks[i]->addSocket(sock,socketTimeOut);
      intasks[i]->notify();
      outtasks[i]->notify();

      return;
    }
  }
  SmppOutputThread *out=new SmppOutputThread(smeManager);
  SmppInputThread *in=new SmppInputThread(smeManager);

  in->setInactivityTime(inactivityTime);
  in->setInactivityTimeOut(inactivityTimeOut);
  out->setInactivityTime(inactivityTime);
  out->setInactivityTimeOut(inactivityTimeOut);


  out->assignIn(in);
  in->assignOut(out);

  sock->setData(0,(void*)2);

  in->addSocket(sock,socketTimeOut);
  out->addSocket(sock,socketTimeOut);

  intasks.Push(in);
  outtasks.Push(out);

  tp->startTask(out);
  tp->startTask(in);

}

int SmppSocketsManager::removeSocket(Socket* sock)
{
  MutexGuard g(mtxRemove);
  int x=(int)sock->getData(0);
  x--;
  trace2("socket ref count for %p:%d",sock,x);
  sock->setData(0,(void*)x);
  if(x)return x;
  trace2("deleting socket:%p",sock);
  sock->Abort();
  delete sock;
  return 0;
}

};//smppio
};//system
};//smsc
