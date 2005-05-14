/*
  $Id$
*/

#include "smsc.hpp"
#include <memory>
#include <vector>
#ifndef linux
#include <thread.h>
#endif

#include <exception>

#include "util/udh.hpp"

namespace smsc{
namespace scag{

#define WAIT_DATA_TIMEOUT 100 /* ms */
using smsc::smeman::CommandId;
using smsc::smeman::SmscCommand;
using std::exception;
using smsc::sms::SMS;
using smsc::smeman::SmeIterator;
using std::auto_ptr;

#define __CMD__(x) smsc::smeman::x

bool Smsc::routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,
  smsc::router::RouteInfo* ri,SmeIndex idx)
{
  proxy = 0;
  bool ok ;
  if(idx==-1)
  {
    ok=getRouterInstance()->lookup(org,dst,proxy,&dest_idx,ri);
  }else
  {
    ok=getRouterInstance()->lookup(idx,org,dst,proxy,&dest_idx,ri);
  }
  return ok;
}


void Smsc::mainLoop()
{
  typedef std::vector<SmscCommand> CmdVector;
  CmdVector frame;
  Event e;
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.mainLoop");
#ifndef linux
  thr_setprio(thr_self(),127);
#endif
  time_t lastKillTrCmd=time(NULL);
  for(;;)
  {
    do
    {

      smeman.getFrame(frame,WAIT_DATA_TIMEOUT,false);
      if ( stopFlag ) return;

      time_t now=time(NULL);
      if(now-lastKillTrCmd>2)
      {
        eventqueue.enqueue(SmscCommand::makeKillExpiredTransactions());
        lastKillTrCmd=now;
      }

    }while(!frame.size());


    ////
    //
    // here will be event queue limit check!
    //

    //
    //
    //
    //////


    int submitCount=0;
    for(CmdVector::iterator i=frame.begin();i!=frame.end();i++)
    {
      try{
        int prio=i->getProxy()->getPriority()/1024;
        if(prio<0)prio=0;
        if(prio>=32)prio=31;
        (*i)->set_priority(prio);
        (*i)->sourceId=i->getProxy()->getSystemId();
      }catch(exception& e)
      {
        __trace2__("Source proxy died after selection: %s",e.what());
#ifdef __GNUC__
        CmdVector::difference_type pos=std::distance(frame.begin(),i);
#else
        CmdVector::difference_type pos;
        std::distance(frame.begin(),i,pos);
#endif
        frame.erase(i);
        i=frame.begin()+pos;
        i--;
        continue;
      }catch(...)
      {
        __trace__("Source proxy died after selection");

#ifdef __GNUC__
        CmdVector::difference_type pos=std::distance(frame.begin(),i);
#else
        CmdVector::difference_type pos;
        std::distance(frame.begin(),i,pos);
#endif
        frame.erase(i);
        i=frame.begin()+pos;
        i--;
        continue;
      }
      if((*i)->get_commandId()==SUBMIT || (*i)->get_commandId()==FORWARD)
      {
        submitCount++;
      }else
      {
        try{
          processCommand((*i));
        }catch(...)
        {
          __warning2__("command processing failed:%d",(*i)->get_commandId());
        }
      }
    }

    if(submitCount==0)
    {
      continue; //start cycle from start
    }

    // main "delay" cycle

    while(frame.size())
    {

      int equnsize;
      eventqueue.getStats(equnsize);

      if(equnsize+1<=eventQueueLimit)
      {
        SmscCommand cmd=frame.back();
        frame.pop_back();
        if(cmd->get_commandId()==SUBMIT || cmd->get_commandId()==FORWARD)
        {
          try{
            processCommand(cmd);
          }catch(...)
          {
            __warning2__("command processing failed:%d",cmd->get_commandId());
          }
        }
        continue;
      }
#ifdef linux
      typedef timespec timestruc_t;
#endif

      timestruc_t tv={0,1000000};
      nanosleep(&tv,0);
    }

  } // end of main loop
}


void Smsc::generateAlert(SMS* sms)
{
  eventqueue.enqueue(SmscCommand::makeAlert(sms));
}


void Smsc::processCommand(SmscCommand& cmd)
{
  switch(cmd->get_commandId())
  {
    case __CMD__(REPLACE):
    {
      int pos;
      SMSId id;
      if(sscanf(cmd->get_replaceSm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
         cmd->get_replaceSm().messageId.get()[pos]!=0)
      {
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeReplaceSmResp
          (
            cmd->get_dialogId(),
            smsc::system::Status::INVMSGID
          )
        );
        return;
      };
      break;
    }
    case __CMD__(QUERY):
    {
      int pos;
      SMSId id;
      if(sscanf(cmd->get_querySm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
         cmd->get_querySm().messageId.get()[pos]!=0)
      {
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeQuerySmResp
          (
            cmd->get_dialogId(),
            smsc::system::Status::INVMSGID,
            0,0,0,0
          )
        );
        return;
      };
      break;
    }
    case __CMD__(CANCEL):
    {
      SMSId id;
      if((cmd->get_cancelSm().messageId.get() && cmd->get_cancelSm().serviceType.get()))
      {
          cmd.getProxy()->putCommand
          (
            SmscCommand::makeCancelSmResp
            (
              cmd->get_dialogId(),
              smsc::system::Status::CANCELFAIL
            )
          );
          return;
      }

      if(cmd->get_cancelSm().messageId.get())
      {
        int pos=0;
        if(sscanf(cmd->get_cancelSm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
           cmd->get_cancelSm().messageId.get()[pos]!=0)
        {
          cmd.getProxy()->putCommand
          (
            SmscCommand::makeCancelSmResp
            (
              cmd->get_dialogId(),
              smsc::system::Status::INVMSGID
            )
          );
          return;
        };
      }else
      {
        //multicancel not supported
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeCancelSmResp
          (
            cmd->get_dialogId(),
            smsc::system::Status::CANCELFAIL
          )
        );
        return;
      }
      break;
    }
    case __CMD__(SMEALERT):return;
    default:;
  }
  eventqueue.enqueue(cmd);
}


}
}
