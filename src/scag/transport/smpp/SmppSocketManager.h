#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER_H__

#include "SmeAcceptor.h"
#include "SmscConnector.h"
#include "SmppReader.h"
#include "SmppWriter.h"
#include "core/threads/ThreadPool.hpp"
#include "SmppSMInterface.h"

namespace scag{
namespace transport{
namespace smpp{

class SmppSocketManager:public SmppSMInterface{
public:
  SmppSocketManager()
  {
    acc=new SmeAcceptor(this);
    conn=new SmscConnector(this);
    tp.startTask(acc);
  }
  void registerSocket(SmppSocket* sock);
  void unregisterSocket(SmppSocket* sock);
  void shutdown();
protected:
  enum{MaxSocketsPerThread=16};
  Array<SmppReader*> readers;
  Array<SmppWriter*> writers;
  sync::Mutex mtx;

  SmeAcceptor* acc;
  SmscConnector* conn;


  thr::ThreadPool tp;
};


}//smpp
}//transport
}//scag


#endif
