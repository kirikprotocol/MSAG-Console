#include "system/status_sme.hpp"
#include "util/smstext.h"
#include "system/version.h"

namespace smsc{
namespace system{

using namespace smsc::util;

static int strprintf(string& s,const char* fmt,...)
{
  va_list arglist;
  int sz=1024;
  auto_ptr<char> buf(new char[sz]);
  va_start(arglist,fmt);
  int size=vsnprintf( buf.get(), sz,fmt,arglist);
  va_end(arglist);
  if(size<0)
  {
    sz+=-size+16;
    buf=auto_ptr<char>(new char[sz]);
    va_end(arglist);
    va_start(arglist,fmt);
    vsnprintf( buf.get(), sz,fmt,arglist);
  }
  va_end(arglist);
  s=buf.get();
  return size;
}

int StatusSme::Execute()
{
  SmscCommand cmd,resp;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
  SMS *sms;
  SMS s;
  //string answer;
  char msc[]="";
  char imsi[]="";
  s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  s.setDeliveryReport(0);
  s.setArchivationRequested(false);
  s.setEServiceType(servType.c_str());
  s.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smsc.system.StatusSme");

  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid==smsc::smeman::SUBMIT_RESP)continue;
    if(cmd->cmdid!=smsc::smeman::DELIVERY)
    {
      __trace2__("StatusSme: incorrect command submitted:%d",cmd->cmdid);
      smsc_log_warn(log, "Incorrect command received");
      continue;
    }
    sms=cmd->get_sms();
    resp=SmscCommand::makeDeliverySmResp(
      sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
      cmd->get_dialogId(),
      Status::OK);
    putIncomingCommand(resp);

    getSmsText(sms,body,sizeof(body));
    string request=body;
    string answer;

    string arg;
    string::size_type pos=request.find(' ');
    if(pos!=string::npos)
    {
      arg=request.substr(pos+1);
      request.erase(pos);
    }

    if(request=="scheduler" || request=="sc")
    {
      Scheduler::SchedulerCounts cnts;
      smsc->getScheduler()->getCounts(cnts);
      char buf[256];
      time_t now=time(NULL);
      time_t headTime=smsc->scheduler->tlHeadTime();
      sprintf(buf,"tlc:%d,ftc=%d,tll:%d,rs=%d,ipc:%d\nht/now=%d/%d(",
        cnts.timeLineCount,
        cnts.firstTimeCount,
        cnts.timeLineSize,
        cnts.regSize,
        cnts.inProcCount,
        headTime,
        now
      );
      answer=buf;
      ctime_r(&headTime,buf);
      buf[strlen(buf)-1]=0;
      answer+=buf;
      answer+='/';
      ctime_r(&now,buf);
      buf[strlen(buf)-1]=0;
      answer+=buf;
      answer+=')';
    }else if (request=="eventqueue" || request=="eq")
    {
      int cnt=smsc->eventqueue.getCounter();
      int eqhash,equnl;
      smsc->eventqueue.getStats(eqhash,equnl);
      char buf[128];
      sprintf(buf,"cnt=%d, locked=%d, unlocked=%d",cnt,eqhash,equnl);
      answer=buf;
    }else if (request=="tasks")
    {
      char buf[64];
      sprintf(buf,"%d tasks in task container",smsc->tasks.tasksCount);
      answer=buf;
    }else if (request=="tc")
    {
      if(arg.length()==0)
      {
        int cnt;
        {
          MutexGuard g(smsc->tcontrol->mtx);
          cnt=smsc->tcontrol->totalCounter.Get();
        }
        char buf[64];
        sprintf(buf,"Current load:%lf",(double)cnt/smsc->tcontrol->cfg.shapeTimeFrame);
        answer=buf;
      }else
      {
        SmeIndex idx=smsc->getSmeIndex(arg);
        if(idx==INVALID_SME_INDEX)
        {
          answer="unknown sme";
        }else
        {
          int dcnt,rcnt;
          {
            MutexGuard g(smsc->tcontrol->mtx);
            dcnt=smsc->tcontrol->getTSC(smsc->tcontrol->deliverCnt,idx)->Get();
            rcnt=smsc->tcontrol->getTSC(smsc->tcontrol->responseCnt,idx)->Get();
          }
          char buf[64];
          int ptf=smsc->tcontrol->cfg.protectTimeFrame;
          sprintf(buf,"deliverCnt=%lf, receivedCnt=%lf",(double)dcnt/ptf,(double)rcnt/ptf);
          answer=buf;
        }
      }
    }else if(request=="getsched")
    {
      if(arg.length()==0)
      {
        answer="usage: getsched address";
      }else
      {
        time_t sc=smsc->scheduler->getScheduleByAddr(arg.c_str());
        if(sc==0)
        {
          answer="chain for "+arg+" not found in scheduled";
        }else
        {
          strprintf(answer,"chain scheduled to %u",sc);
        }
      }
    }else if(request=="hlralert")
    {
      try{
        putIncomingCommand(SmscCommand::makeHLRAlert(arg.c_str()));
        answer="hlr alert generated";
      }catch(...)
      {
        answer="invalid address";
      }
    }else if(request=="ver")
    {
      answer=getStrVersion();
    }
    else
    {
      answer="unknown command";
    }


    time_t t=time(NULL)+60;
    s.setValidTime(t);
    s.setOriginatingAddress(sms->getDestinationAddress());
    s.setDestinationAddress(sms->getOriginatingAddress());
    s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    fillSms(&s,answer.c_str(),answer.length(),CONV_ENCODING_CP1251,DataCoding::UCS2);

    s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
      sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));

    resp=SmscCommand::makeSumbmitSm(s,getNextSequenceNumber());
    putIncomingCommand(resp);
  }
  return 0;
}
}//system
}//smsc
