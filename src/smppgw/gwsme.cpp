#include "gwsme.hpp"
#include "util/sleep.h"
#include "util/debug.h"

namespace smsc{
namespace smppgw{

int GatewaySme::Execute()
{
  while(!isStopping)
  {
    try{
      if(cfgIdx>1)cfgIdx=0;
      sesscfg.host=hosts[cfgIdx];
      sesscfg.port=ports[cfgIdx];
      info2(log,"Connecting to %s:%d, systemId=%s",sesscfg.host.c_str(),sesscfg.port,sesscfg.sid.c_str());
      sess.setConfig(sesscfg);
      sess.connect();
    }catch(smsc::sme::SmppConnectException& e)
    {
      SmscCommand cmd;
      {
        MutexGuard gIn(mutexout);
        MutexGuard gOut(mutexin);
        while(outqueue.Count()>0)
        {
          outqueue.Pop(cmd);
          if(cmd->get_commandId()==SUBMIT)
          {
            inqueue.Push(SmscCommand::makeSubmitSmResp("0",cmd->get_dialogId(),smsc::system::Status::SYSERR));
          }
        }
      }

      warn2(log,"failed to connect to smsc at %s:%d, reason - %s",sesscfg.host.c_str(),sesscfg.port,e.what());
      smsc::util::millisleep(2000);
      cfgIdx++;
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
        while(!isStopping && isConnected() && outqueue.Count()==0)
        {
          mutexout.wait(1000);
        }
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
    debug1(log,"closing session");
    sess.close();
    cfgIdx++;
  }
  debug1(log,"finishing gwsme");
  return 0;
}


}//namespace smppgw
}//namespace smsc
