#include "system/smppio/SmppSocketsManager.hpp"
#include "system/smppio/SmppIOTask.hpp"
#include <string.h>

namespace smsc{
namespace system{
namespace smppio{

void SmppSocketsManager::registerSocket(Socket* sock)
{
  {
    MutexGuard mg(mtxRemove);
    char buf[32];
    sock->GetPeer(buf);
    char* ptr=strchr(buf,':');
    if(ptr)*ptr=0;
    int* cntptr=connectCnt.GetPtr(buf);
    int lim=defaultConnectLimit;
    int* limptr=connectLim.GetPtr(buf);
    if(limptr)
    {
      lim=*limptr;
    }
    int cnt=1;
    if(cntptr)
    {
      cnt=1+*cntptr;
    }
    info2(log,"Connect from %s, connectuions count=%d, limit=%d",buf,cnt,lim);
    if(cnt>lim)
    {
      warn2(log,"Aborting connection from %s - connections limit reached (%d)",buf,lim);
      sock->Abort();
      delete sock;
      return;
    }

    if(cntptr)
    {
      (*cntptr)++;
    }else
    {
      connectCnt.Insert(buf,1);
    }
    ptr2addr.insert(Ptr2AddrMap::value_type(sock,buf));
  }
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
  Ptr2AddrMap::iterator it=ptr2addr.find(sock);
  connectCnt.Get(it->second.c_str())--;
  info2(log,"Decrease connections count for %s(%d)",it->second.c_str(),connectCnt.Get(it->second.c_str()));
  ptr2addr.erase(it);

  info2(log,"deleting socket:%p",sock);
  sock->Abort();
  delete sock;
  return 0;
}

}//smppio
}//system
}//smsc
