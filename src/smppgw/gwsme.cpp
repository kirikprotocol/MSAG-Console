#include "gwsme.hpp"
#include "util/sleep.h"

namespace smsc{
namespace smppgw{

int GatewaySme::Execute()
{
  while(!isStopping)
  {
    try{
      sess.connect();
    }catch(smsc::sme::SmppConnectException& e)
    {
      warn2(log,"failed to connect to smsc at %s:%d, reason - %s",sesscfg.host.c_str(),sesscfg.port,e.what());
      smsc::util::millisleep(2000);
      continue;
    }
    debug1(log,"connected to smsc");
    connected=true;
    smsc::sme::SmppTransmitter* trans=sess.getAsyncTransmitter();
    while(!isStopping)
    {
      SmscCommand cmd;
      {
        MutexGuard g(mutexout);
        while(!isStopping && outqueue.Count()==0)mutexout.wait(1000);
        if(isStopping || !isConnected())break;
        outqueue.Pop(cmd);
      }
      if(!isConnected())break;
      SmppHeader* pdu=0;
      try{
        pdu=cmd.makePdu();
      }catch(...)
      {
        warn2(log,"failed to construct pdu from command:%d",cmd->get_commandId());
        continue;
      }
      /*
      if((pdu->get_commandId()&0x80000000UL)==0)
      {
        pdu->set_sequenceNumber(sess.getNextSeq());
      }
      */
      debug2(log,"sending pdu to smsc:%#X",pdu->get_commandId());
      try{
        trans->sendPdu(pdu);
      }catch(...)
      {
        warn2(log,"failed to send pdu %#X",pdu->get_commandId());
      }
      disposePdu(pdu);
    }
  }
  return 0;
}


}//namespace smppgw
}//namespace smsc
