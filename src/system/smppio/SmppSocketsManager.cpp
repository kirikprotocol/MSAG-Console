#include "system/smppio/SmppSocketsManager.hpp"
#include "system/smppio/SmppIOTask.hpp"

namespace smsc{
namespace system{
namespace smppio{

void SmppSocketsManager::registerSocket(Socket* sock)
{
  int i;
  for(i=0;i<intasks.Count();i++)
  {
    if(intasks[i]->socketsCount()<SM_SOCK_PER_THREAD)
    {
      intasks[i]->addSocket(sock);
      outtasks[i]->addSocket(sock);
      return;
    }
  }
  SmppOutputThread *out=new SmppOutputThread;
  SmppInputThread *in=new SmppInputThread(smeManager);

  out->assignIn(in);
  in->assignOut(out);

  in->addSocket(sock);
  out->addSocket(sock);

  intasks.Push(in);
  outtasks.Push(out);

  tp->startTask(out);
  tp->startTask(in);

}

};//smppio
};//system
};//smsc
