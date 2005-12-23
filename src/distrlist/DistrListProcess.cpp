// $Id$

#include "DistrListProcess.h"
#include "../system/status.h"
#include "util/smstext.h"
#include <string>
#include <ctype.h>
#define DLP_TIMEOUT 1000
#define WAIT_SUBMISSION (8)

#define __FUNCTION__ __func__

//#define DLSMSADMIN

namespace smsc{
namespace distrlist{

using namespace smsc::system;

static uint32_t GetNextDialogId()
{
  static uint32_t id = 0;
  return ++id;
}

struct BIG_MULTI{};
struct INV_DSTADDR{};

DistrListProcess::DistrListProcess(DistrListAdmin* admin,SmeRegistrar* reg) :
  admin(admin),
  managerMonitor(0),
  seq(1),
  smereg(reg)
{}

DistrListProcess::~DistrListProcess()
{
  smereg->unregisterSmeProxy("DSTRLST");
}
void DistrListProcess::close()  {}

/// кидает exception если был достигнут лимит
/// и длина очереди еще не упала до допустимого значения
void DistrListProcess::putCommand(const SmscCommand& cmd)
{
  MutexGuard g(mon);
  mon.notify();
  outQueue.Push(cmd);
}

bool DistrListProcess::getCommand(SmscCommand& cmd)
{
  MutexGuard g(mon);
  if(inQueue.Count()==0)return false;
  inQueue.Shift(cmd);
  return true;
}

SmeProxyState DistrListProcess::getState() const
{
  return state;
}

void DistrListProcess::init()
{
  __trace__("DistrListProcess: init");
  managerMonitor=0;
  state=VALID;
  seq=1;
}

//SmeProxyPriority DistrListProcess::getPriority() {return SmeProxyPriorityDefault;}
//void DistrListProcess::setPriority(SmeProxyPriority) {}
bool DistrListProcess::hasInput() const
{
  MutexGuard g(mon);
  return inQueue.Count()!=0;
}

// for detach monitor call with NULL
void DistrListProcess::attachMonitor(ProxyMonitor* monitor)
{
  managerMonitor=monitor;
}

// check what proxy has attached monitor (proxy is attached to dispatcher)
bool DistrListProcess::attached()
{
  return managerMonitor!=0;
}

uint32_t DistrListProcess::getNextSequenceNumber()
{
  MutexGuard g(mon);
  return ++seq;
}


static void lowercase(string& str)
{
  for(int i=0;i<str.length();i++)str[i]=tolower(str[i]);
}

//uint32_t DistrListProcess::getUniqueId() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
//uint32_t DistrListProcess::getIndex() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
//unsigned long DistrListProcess::getPreferredTimeout() { return 8; }
//const std::string& DistrListProcess::getSourceAddressRange(){return nullstr;};
const char * DistrListProcess::getSystemId() const
{
  return "DSTRLST";
}

// ThreadedTask
int DistrListProcess::Execute()
{
  SmscCommand cmd,resp,answ;
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smsc.distrlist.process");
  while(!isStopping)
  {
    CheckTimeouts();
    if(!hasOutput())
    {
      waitFor(DLP_TIMEOUT);
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if ( cmd->get_commandId() == SUBMIT_MULTI_SM )
    {
      try{
        SubmitMulti(cmd);
      }catch(exception& e){
        __warn2__(smsc::logger::_trace_cat, ":DPL: <exception> %s", e.what());
        SmscCommand cmdR = SmscCommand::makeSubmitMultiResp(0,cmd->get_dialogId(),Status::CNTSUBDL);
        cmd.getProxy()->putCommand(cmdR);
      }catch(BIG_MULTI&){
        __warn__(smsc::logger::_trace_cat, ":DPL: <exception> counts of member of multi great then MAX_COUNT for task");
        SmscCommand cmdR = SmscCommand::makeSubmitMultiResp(0,cmd->get_dialogId(),Status::CNTSUBDL);
        cmd.getProxy()->putCommand(cmdR);
      }catch(INV_DSTADDR&){
        __warn__(smsc::logger::_trace_cat, ":DPL: <exception> one of destination addresses is invalid");
        SmscCommand cmdR = SmscCommand::makeSubmitMultiResp(0,cmd->get_dialogId(),Status::INVDSTADR);
        cmd.getProxy()->putCommand(cmdR);
      }
    }
    else if ( cmd->get_commandId() == DELIVERY )
    {
      SMS &sms=*cmd->get_sms();
      {
        SmscCommand resp=SmscCommand::makeDeliverySmResp("",cmd->get_dialogId(),Status::OK);
        putIncomingCommand(resp);
      }
      string s;
      smsc::util::getSmsText(&sms,s);
      string::size_type sppos=s.find(' ');
      string arg;
      string arg2;
      string::size_type argend=string::npos;
      string cmd;
      if(sppos!=string::npos)
      {
        cmd.assign(s,0,sppos);
        string::size_type i=sppos+1;
        while(i<s.length() && s.at(i)==' ')i++;
        string::size_type j=i;
        while(i<s.length() && s.at(i)!=' ')i++;
        arg.assign(s,j,i-j);
        argend=i==s.length()?i:i+1;
        if(argend!=s.length())
        {
          arg2.assign(s,argend,string::npos);
        }
      }else
      {
        cmd=s;
      }

      lowercase(cmd);
      lowercase(arg);

      string answer;
      string tmpl="dl.unknowncommand";
      string reason;
      char addr[32];
      sms.getOriginatingAddress().getValue(addr);
      string fullarg=addr;
      fullarg+='/';
      fullarg+=arg;
      if(arg.length()>0 && arg[0]=='/')
      {
        fullarg=arg.substr(1);
      }
      try{
        if(cmd!="send" && arg.find('/')!=string::npos)
        {
          tmpl="dl.invalidcmdparam";
          reason="prefix unexpected";
        }else
        if(cmd=="add")
        {
          try{
            tmpl="dl.adderr";
            admin->addDistrList(fullarg,false,sms.getOriginatingAddress(),0);
            tmpl="dl.addok";
          }catch(ListAlreadyExistsException& e)
          {
            reason="list already exists";
          }catch(PrincipalNotExistsException& e)
          {
            reason="principal not exists";
          }catch(ListCountExceededException& e)
          {
            reason="list count exceeded";
          }
        }else
        if(cmd=="del")
        {
          tmpl="dl.delerr";
          admin->deleteDistrList(fullarg);
          tmpl="dl.delok";
        }else
        if(cmd=="list")
        {
          if(arg.length()>0)
          {
            Array<Address> m;
            try{
              m=admin->members(fullarg,sms.getOriginatingAddress());
            }catch(IllegalSubmitterException& e)
            {
              tmpl="dl.mlistmerr";
              reason="not allowed to list members";
            }
            if(m.Count()==0)
            {
              tmpl="dl.mlistempty";
            }else
            {
              char buf[32];
              for(int i=0;i<m.Count();i++)
              {
                m[i].getText(buf,sizeof(buf));
                if(i!=0)answer+=',';
                answer+=buf;
              }
            }
          }else
          {
            try{
              Array<DistrList> lst=admin->list(sms.getOriginatingAddress());
              if(lst.Count()==0)
              {
                tmpl="dl.listempty";
              }else
              {
                for(int i=0;i<lst.Count();i++)
                {
                  if(i!=0)answer+=',';
                  answer+=lst[i].name;
                }
              }
            }catch(PrincipalNotExistsException& e)
            {
              tmpl="dl.listerr";
              reason="you are not registered as list owner";
            }
          }
        }else
        if(cmd=="addm")
        {
          Address member(arg2.c_str());
          try{
            tmpl="dl.madderr";
            admin->addMember(fullarg,member);
            tmpl="dl.maddok";
          }catch(MemberAlreadyExistsException& e)
          {
            reason="member already exists";
          }
          catch(MemberCountExceededException& e)
          {
            reason="members count limit exceeded";
          }
        }else
        if(cmd=="delm")
        {
          Address member(arg2.c_str());
          try{
            tmpl="dl.mdelerr";
            admin->deleteMember(fullarg,member);
            tmpl="dl.mdelok";
          }catch(MemberNotExistsException& e)
          {
            reason="member doesn't exists";
          }
        }else
        if(cmd=="adds")
        {
          Address submitter(arg2.c_str());
          try{
            tmpl="dl.sadderr";
            admin->grantPosting(fullarg,addr,submitter);
            tmpl="dl.saddok";
          }catch(ListNotExistsException& e)
          {
            reason="list not found";
          }catch(PrincipalNotExistsException& e)
          {
            reason="principal not found";
          }catch(SubmitterAlreadyExistsException& e)
          {
            reason="submitter already exists";
          }
        }else
        if(cmd=="dels")
        {
          Address submitter(arg2.c_str());
          try{
            tmpl="dl.sdelerr";
            admin->revokePosting(fullarg,addr,submitter);
            tmpl="dl.sdelok";
          }catch(ListNotExistsException& e)
          {
            reason="list not found";
          }catch(SubmitterNotExistsException& e)
          {
            reason="submitter not found";
          }catch(IllegalSubmitterException& e)
          {
            reason="illegal submitter";
          }
        }else
        if(cmd=="slist")
        {
          try{
            tmpl="dl.slisterr";
            Array<Address> addrs;
            admin->getSubmitters(fullarg,addrs);
            tmpl="dl.slistok";
            if(addrs.Count()==0)
            {
              tmpl="dl.slistempty";
            }else
            {
              char buf[32];
              for(int i=0;i<addrs.Count();i++)
              {
                addrs[i].getText(buf,sizeof(buf));
                if(i!=0)answer+=',';
                answer+=buf;
              }
            }
          }catch(ListNotExistsException& e)
          {
            reason="list not found";
          }
        }else
        if(cmd=="send")
        {
          Array<Address> m;
          try{
            tmpl="dl.senderr";
            m=admin->members(fullarg,sms.getOriginatingAddress());
            tmpl="dl.sendok";
            SMS newsms=sms;
            if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
            {
              unsigned len;
              const char* msg=sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
              if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
              {
                newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msg+argend*2,len-argend*2);
              }else
              {
                newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msg+argend,len-argend);
              }
            }else
            {
              unsigned len;
              const char* msg=sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
              if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
              {
                newsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msg+argend*2,len-argend*2);
                newsms.setIntProperty(Tag::SMPP_SM_LENGTH,len-argend*2);
              }else
              {
                newsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msg+argend,len-argend);
                newsms.setIntProperty(Tag::SMPP_SM_LENGTH,len-argend);
              }
            }
            for(int i=0;i<m.Count();i++)
            {
              newsms.setDestinationAddress(m[i]);
              SmscCommand snd=SmscCommand::makeSumbmitSm(newsms,GetNextDialogId());
              putIncomingCommand(snd);
            }
            char buf[32];
            sprintf(buf,"%d",m.Count());
            arg2=buf;
          }catch(IllegalSubmitterException& e)
          {
            reason="access denied";
          }
        }
#ifdef DLSMSADMIN
        else if(cmd=="addprincipal")
        {
          Principal p(fullarg.c_str(),10,10);
          admin->addPrincipal(p);
        }
#endif //DLSMSADMIN

      }catch(smsc::core::buffers::FileException& e)
      {
        reason="database error";
      }catch(ListNotExistsException& e)
      {
        reason="list doesn't exists";
      }catch(exception& e)
      {
        reason=e.what();
      }catch(...)
      {
        reason="unknown";
      }
      SMS ans;
      ans.setOriginatingAddress(sms.getDestinationAddress());
      ans.setDestinationAddress(sms.getOriginatingAddress());
      char msc[]="";
      char imsi[]="";
      ans.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
      ans.setDeliveryReport(0);
      ans.setArchivationRequested(false);
      ans.setEServiceType(serviceType.c_str());
      ans.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,0);
      ans.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protocolId);
      ans.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE,
        sms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
      if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==1)
      {
        ans.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,17);
        // clear 0,1 bits and set them to datagram mode
        ans.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,
            (ans.getIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS)&~0x03)|0x01);
      }
      if(answer.length()==0)
      {
        arg=fullarg;
        string::size_type pos=arg.find('"');
        while(pos!=string::npos)
        {
          arg.at(pos)=' ';
          pos=arg.find('"',pos);
        }
        pos=arg2.find('"');
        while(pos!=string::npos)
        {
          arg2.at(pos)=' ';
          pos=arg2.find('"',pos);
        }
        answer="#template="+tmpl+"#{reason}=\""+reason+"\" {arg1}=\""+arg+"\" {arg2}=\""+arg2+"\"";
      }
      smsc::util::fillSms(&ans,answer.c_str(),answer.length(),CONV_ENCODING_CP1251,DataCoding::UCS2|DataCoding::LATIN1);
      SmscCommand cmdAnswer=SmscCommand::makeSumbmitSm(ans,GetNextDialogId());
      putIncomingCommand(cmdAnswer);
    }
    else if ( cmd->get_commandId() == SUBMIT_RESP )
    {
      SubmitResp(cmd);
    }
    else
    {
      __warning2__(":DLP: unknown cmd id %d",cmd->get_commandId());
    }
  }
  return 0;
}

const char* DistrListProcess::taskName() { return "DitrListProcess";}

void DistrListProcess::putIncomingCommand(const SmscCommand& cmd)
{
  mon.Lock();
  inQueue.Push(cmd);
  mon.Unlock();
  managerMonitor->Signal();
}

SmscCommand DistrListProcess::getOutgoingCommand()
{
  MutexGuard g(mon);
  SmscCommand cmd;
  outQueue.Shift(cmd);
  return cmd;
}

bool DistrListProcess::hasOutput()
{
  MutexGuard g(mon);
  return outQueue.Count()!=0;
}

void DistrListProcess::waitFor(unsigned timeout)
{
  mon.Lock();
  mon.wait(timeout);
  mon.Unlock();
}

void DistrListProcess::SubmitMulti(SmscCommand& cmd)
{
  SubmitMultiSm* multi = cmd->get_Multi();
  multi->msg.setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,0);
  if ( multi->number_of_dests == 0 ) {
    __trace__(":DPL: empty multisubmit");
    //return;
    SmscCommand cmd2 = SmscCommand::makeSubmitMultiResp("",cmd->get_dialogId(),Status::INVNUMDESTS);
    cmd2->get_MultiResp()->set_unsuccessCount(0);
    SmeProxy* srcproxy =  cmd.getProxy();
    srcproxy->putCommand(cmd2);
    return;
  }
  auto_ptr<ListTask> task(new ListTask());
  task->count = 0;
  task->cmd = cmd;
  __trace2__(":DPL: numer of dests %d",multi->number_of_dests);
  for ( unsigned i=0; i < multi->number_of_dests; ++i )
  {
    if ( task->count >= MAX_COUNT ) {
      throw BIG_MULTI();
    }
    if ( multi->dests[i].dest_flag ){
      __trace2__(":DPL: distrib list %s",multi->dests[i].value.c_str());
      Array<Address> addresses;
      try{
        addresses = admin->members(multi->dests[i].value,multi->msg.getOriginatingAddress());
      }catch(exception& e){
        __trace2__(":DPL: <exception> : %s",e.what());
        continue;
      }
      unsigned count = addresses.Count();
      if ( count+task->count > MAX_COUNT ) {
        __trace2__(":DPL: members count(%d)+task->count(%d) great then MAX_COUNT for task",
                      count,task->count);
        throw BIG_MULTI();
      }
      for ( unsigned i = 0; i < count; ++i )
      {
        __trace2__(":DPL:MEMBER %d.%d.%s",addresses[i].type,addresses[i].plan,addresses[i].value);
        task->list[task->count].addr = addresses[i];
        task->list[task->count].dialogId = 0;
        task->list[task->count].responsed = false;
        task->list[task->count].errcode = Status::CNTSUBDL;
        ++task->count;
      }
    }
    else
    {
      __trace2__(":DPL: distrib addr %d.%d.%s",multi->dests[i].ton,multi->dests[i].npi,multi->dests[i].value.c_str());
      if( multi->dests[i].value.length() == 0 ) {
        throw INV_DSTADDR();
      }
     task->list[task->count].addr = Address(
       multi->dests[i].value.length(),
       multi->dests[i].ton,
       multi->dests[i].npi,
       multi->dests[i].value.c_str());
     __trace2__(":DPL: %d.%d.%s",multi->dests[i].ton,multi->dests[i].npi,multi->dests[i].value.c_str());
     task->list[task->count].dialogId = 0;
     task->list[task->count].responsed = false;
     task->list[task->count].errcode = Status::CNTSUBDL;
      ++task->count;
    }
  }
  __trace2__(":DPL: count of members %d",task->count);
  task->submited_count = 0;
  if ( task->count != 0 ) {
    for ( unsigned i=0; i<task->count; ++i )
    {
      task->list[i].dialogId = GetNextDialogId();
      SMS& msg = multi->msg;
      msg.setDestinationAddress(task->list[i].addr);
      TPAIR p(task.get(),i);
      task_map.insert( pair<unsigned,TPAIR>(task->list[i].dialogId,p) );
      __trace2__(":DPL:DEST %d.%d.%s",task->list[i].addr.type,task->list[i].addr.plan,task->list[i].addr.value);
      putIncomingCommand(SmscCommand::makeSumbmitSm(msg,task->list[i].dialogId));
      __trace2__(":DPL: task %d of (0x%x:%d) has been scheduled for submit",
        task->list[i].dialogId,
        task.get(),
        task->count);
    }
    task->startTime = time(0);
    task_sheduler.push_back(task.get());
    __trace2__(":DPL: task list 0x%x has been scheduled",task.get());
    task.release();
  }else{
    __trace__(":DPL: sending responce immediate");
    SmscCommand cmd2 = SmscCommand::makeSubmitMultiResp("",task->cmd->get_dialogId(),0);
    cmd2->get_MultiResp()->set_unsuccessCount(0);
    SmeProxy* srcproxy =  task->cmd.getProxy();
    srcproxy->putCommand(cmd2);
  }
}

void DistrListProcess::SubmitResp(SmscCommand& cmd)
{
  /*map<pair<unsigned,pair<ListTask*,unsigned> > >*/
  __trace2__(":DPL: %s",__FUNCTION__);
  MAPTYPE::iterator it = task_map.find(cmd->get_dialogId());
  if ( it != task_map.end() ) {
    TPAIR taskpair = it->second;
    if ( taskpair.second >= taskpair.first->count ) {
      __warning__(":DPL: out of adresses range");
      // за пределом массива адресов !!!
    }else{
      ListTask* task = taskpair.first;
      task->list[taskpair.second].responsed = true;//cmd->get_resp()->get_status() == 0;
      task->list[taskpair.second].errcode = cmd->get_resp()->get_status();
      task_map.erase(task->list[taskpair.second].dialogId);
      task->submited_count++;
      __trace2__(":DPL: task %d of (0x%x:%d:%d) has been submited",
        cmd->get_dialogId(),
        task,
        task->count,
        task->submited_count);
      if ( task->submited_count == task->count ) {
        __trace__(":DPL: send submit responce");
        SendSubmitResp(task);
      }
    }
  }else{
    __trace2__(":DPL: has no task with dialogid %d",cmd->get_dialogId());
  }
}

void DistrListProcess::SendSubmitResp(ListTask* task) // удаляет из списка и мапы
{
  __trace2__(":DPL: %s",__FUNCTION__);
  unsigned status = 0;
  if ( task->count != task->submited_count ) status = Status::CNTSUBDL; /// !!!!!! must be fixed
  __trace2__(":DPL: sending submit multi responce, status %d (%d smses not responsed)",status,task->count-task->submited_count);
  SmscCommand cmd = SmscCommand::makeSubmitMultiResp("",task->cmd->get_dialogId(),status);
//  if ( status != 0 ) {
    UnsuccessElement* ue = cmd->get_MultiResp()->get_unsuccess();
    unsigned uno = 0;
    for ( unsigned i=0; i < task->count; ++i )
    {
      if ( task->list[i].errcode != 0 ) {
        __require__ ( uno  < 256 );
        if (!task->list[i].responsed) task_map.erase(task->list[i].dialogId); // остальные уже удалены
        ue[uno].addr = task->list[i].addr;
        ue[uno].errcode = task->list[i].errcode; //0; // !!!!! must be fixed
        ++uno;
      }
    }
    __trace2__(":DPL: unsoccess %d",uno);
    cmd->get_MultiResp()->set_unsuccessCount(uno);
  //}
  LISTTYPE::iterator it = find(task_sheduler.begin(),task_sheduler.end(),task);
  if ( it != task_sheduler.end() ) { task_sheduler.erase(it);  }
  //putIncomingCommand(cmd);
  SmeProxy* srcproxy =  task->cmd.getProxy();
  srcproxy->putCommand(cmd);
}

void DistrListProcess::CheckTimeouts()
{
  time_t curTime = time(0);
  while ( !task_sheduler.empty() && (task_sheduler.front()->startTime+WAIT_SUBMISSION < curTime) )
  {
    //__trace2__(":DPL: T:%s task 0x%x(T:%s) was time out",
    //  ctime(&curTime),task_sheduler.front(),ctime(&task_sheduler.front()->startTime));
    SendSubmitResp(task_sheduler.front()); // удаляет из списка и мапы
  }
}

}} // namespaces
