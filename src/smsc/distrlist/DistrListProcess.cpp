// $Id$

#include <string>
#include <ctype.h>

#include "smsc/status.h"
#include "util/smstext.h"
#include "smsc/resourcemanager/ResourceManager.hpp"

#ifdef SMSEXTRA
#include "smsc/extra/ExtraBits.hpp"
#endif

#include "DistrListProcess.h"
#include <sys/types.h>

#define DLP_TIMEOUT 1000
#define WAIT_SUBMISSION (60)


#define __FUNCTION__ __func__

//#define DLSMSADMIN

namespace smsc{
namespace distrlist{

using namespace smsc;

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
  smereg(reg),
  profiler(0)
{
  autoCreatePrincipal=false;
  defaultMaxLists=10;
  defaultMaxElements=10;
  sendSpeed=5;
}

DistrListProcess::~DistrListProcess()
{
  smereg->unregisterSmeProxy(this);
}
void DistrListProcess::close()  {}

///  exception
///
void DistrListProcess::putCommand(const SmscCommand& cmd)
{
  MutexGuard g(mon);
  mon.notify();
  outQueue.Push(cmd);
}

bool DistrListProcess::getCommand(SmscCommand& cmd)
{
  MutexGuard g(mon);
  if(inQueue.empty())return false;
  InQueue::iterator b=inQueue.begin();
  if(b->first>time(NULL))return false;
  cmd=b->second;
  inQueue.erase(b);
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
  return !inQueue.empty() && inQueue.begin()->first<time(NULL);
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
  for(int i=0;i<str.length();i++)str[i]=tolower((unsigned char)str[i]);
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
  SmscCommand cmd,answ;
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
        putIncomingCommand(resp,0);
      }
      string s;
      smsc::util::getSmsText(&sms,s);
      string::size_type sppos=s.find(' ');
      string arg;
      string arg2;
      string::size_type argend=string::npos;
      string cmdstr;
      if(sppos!=string::npos)
      {
        cmdstr.assign(s,0,sppos);
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
        cmdstr=s;
      }

      lowercase(cmdstr);
      lowercase(arg);

      bool smeMode=false;
      if(originatingAddressForSme.length() && sms.getDestinationAddress().toString()==originatingAddressForSme)
      {
        smeMode=true;
      }

      string answer;
      string tmpl="dl.unknowncommand";
      string reason;
      char addr[32];
      sms.getOriginatingAddress().getValue(addr);
      string fullarg;
      if(arg.find('/')==std::string::npos)
      {
        fullarg=addr;
        fullarg+='/';
        fullarg+=arg;
      }else
      {
        fullarg=arg;
      }
      if(arg.length()>0 && arg[0]=='/')
      {
        fullarg=arg.substr(1);
      }
      enum DlOp{
        dloNone,
        dloAddList,
        dloRemoveList,
        dloCopyList,
        dloRenameList,
        dloAddMember,
        dloRemoveMember,
        dloAddSubmitter,
        dloRemoveSubmitter
      };
      DlOp op=dloNone;
      std::string tmplOk,tmplErr;
      try{
        if(cmdstr!="send" && arg.find('/')!=string::npos)
        {
          tmpl="dl.invalidcmdparam";
          reason="prefix_unexpected";
        }else
        if(cmdstr=="add")
        {
          if(arg.length()==0 || arg.length()>18)
          {
            tmpl="dl.adderr";
            reason="invalid_list_name";
          }
          else
          {
            if(autoCreatePrincipal)
            {
              try{
                admin->getPrincipal(sms.getOriginatingAddress());
              }catch(PrincipalNotExistsException& e)
              {
                Principal p;
                p.address=sms.getOriginatingAddress();
                p.maxLst=defaultMaxLists;
                p.maxEl=defaultMaxElements;
                admin->addPrincipal(p);
              }
            }
            try{
              tmpl="dl.adderr";
              admin->addDistrList(fullarg,false,sms.getOriginatingAddress(),0);
              tmpl="dl.addok";
            }catch(ListAlreadyExistsException& e)
            {
              reason="list_already_exists";
            }catch(PrincipalNotExistsException& e)
            {
              reason="principal_not_exists";
            }catch(ListCountExceededException& e)
            {
              reason="list_count_exceeded";
            }
          }
        }else
        if(cmdstr=="del")
        {
          tmpl="dl.delerr";
          admin->deleteDistrList(fullarg);
          tmpl="dl.delok";
        }else
        if(cmdstr=="list")
        {
          if(arg.length()>0)
          {
            Array<Address> m;
            tmpl="dl.mlisterr";
            try{
              m=admin->members(fullarg,sms.getOriginatingAddress());
            }catch(IllegalSubmitterException& e)
            {
              reason="not_allowed_to_list_members";
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
              reason="not_registered_list_owner";
            }
          }
        }else
        if(cmdstr=="copy")
        {
          tmpl="dl.copyerr";
          try
          {
            std::string newListName=addr;
            newListName+='/';
            newListName+=arg2;
            if(arg2.length()==0 || newListName.length()>30)
            {
              reason="invalid_list_name";
            }else
            {
              admin->copyDistrList(fullarg,newListName);
              tmpl="dl.copyok";
            }
          }catch(ListCountExceededException& e)
          {
            reason="list_count_exceeded";
          }catch(ListAlreadyExistsException& e)
          {
            reason="list_already_exists";
          }
        }else
        if(cmdstr=="rename")
        {
          tmpl="dl.renameerr";
          try
          {
            std::string newListName=addr;
            newListName+='/';
            newListName+=arg2;
            if(arg2.length()==0 || newListName.length()>30)
            {
              reason="invalid_list_name";
            }else
            {
              admin->renameDistrList(fullarg,newListName);
              tmpl="dl.renameok";
            }
          } catch(ListAlreadyExistsException& e)
          {
            reason="list_already_exists";
          }
        }else
        if(cmdstr=="addm")
        {
          tmpl="dl.madderr";
          try{
            Address member(dealias(arg2));
            admin->addMember(fullarg,member);
            tmpl="dl.maddok";
          }catch(MemberAlreadyExistsException& e)
          {
            reason="member_already_exists";
          }
          catch(MemberCountExceededException& e)
          {
            reason="members_count_limit_exceeded";
          }
          catch(std::exception& e)
          {
            reason="invalid_or_empty_address";
          }
        }else
        if(cmdstr=="delm")
        {
          tmpl="dl.mdelerr";
          try{
            Address member(dealias(arg2));
            admin->deleteMember(fullarg,member);
            tmpl="dl.mdelok";
          }catch(MemberNotExistsException& e)
          {
            reason="member_doesnt_exists";
          }
          catch(std::exception& e)
          {
            reason="invalid_or_empty_address";
          }
        }else
        if(cmdstr=="adds")
        {
          tmpl="dl.sadderr";
          try{
            Address submitter(dealias(arg2));
            admin->grantPosting(fullarg,addr,submitter);
            tmpl="dl.saddok";
          }catch(ListNotExistsException& e)
          {
            reason="list_not_found";
          }catch(PrincipalNotExistsException& e)
          {
            reason="principal_not_found";
          }catch(SubmitterAlreadyExistsException& e)
          {
            reason="submitter_already_exists";
          }
          catch(std::exception& e)
          {
            reason="invalid_or_empty_address";
          }
        }else
        if(cmdstr=="dels")
        {
          tmpl="dl.sdelerr";
          try{
            Address submitter(dealias(arg2));
            admin->revokePosting(fullarg,addr,submitter);
            tmpl="dl.sdelok";
          }catch(ListNotExistsException& e)
          {
            reason="list_not_found";
          }catch(SubmitterNotExistsException& e)
          {
            reason="submitter_not_found";
          }catch(IllegalSubmitterException& e)
          {
            reason="illegal_submitter";
          }
          catch(std::exception& e)
          {
            reason="invalid_or_empty_address";
          }
        }else
        if(cmdstr=="slist")
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
            reason="list_not_found";
          }
        }else
        if(cmdstr=="send")
        {
          Array<Address> m;
          try{
            tmpl="dl.senderr";
            m=admin->members(fullarg,sms.getOriginatingAddress());
            //tmpl="dl.sendok";
            tmpl="";
            SMS newsms=sms;
            if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
            {
              unsigned len;
              const char* msg=sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
              if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
              {
                newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msg+argend*2,(unsigned)(len-argend*2));
              }else
              {
                newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msg+argend,(unsigned)(len-argend));
              }
            }else
            {
              unsigned len;
              const char* msg=sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
              if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
              {
                newsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msg+argend*2,(unsigned)(len-argend*2));
                newsms.setIntProperty(Tag::SMPP_SM_LENGTH,(unsigned)(len-argend*2));
              }else
              {
                newsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msg+argend,(unsigned)(len-argend));
                newsms.setIntProperty(Tag::SMPP_SM_LENGTH,(unsigned)(len-argend));
              }
            }

            if(sms.hasStrProperty(Tag::SMSC_IMSI_ADDRESS) || sms.hasStrProperty(Tag::SMSC_MSC_ADDRESS))
            {
              newsms.setOriginatingDescriptor
              (
                (uint8_t)sms.getStrProperty(Tag::SMSC_MSC_ADDRESS).length(),
                sms.getStrProperty(Tag::SMSC_MSC_ADDRESS).c_str(),
                (uint8_t)sms.getStrProperty(Tag::SMSC_IMSI_ADDRESS).length(),
                sms.getStrProperty(Tag::SMSC_IMSI_ADDRESS).c_str(),
                0
              );
            }else
            {
              newsms.setOriginatingDescriptor(sms.getOriginatingDescriptor());
            }

#ifdef SMSEXTRA
            newsms.setIntProperty(Tag::SMSC_EXTRAFLAGS,smsc::extra::EXTRA_GROUPS);
#endif
            std::auto_ptr<ListTask> task(new ListTask);
            task->cmd=cmd;
            task->count=0;
            task->taskType=ttDistrList;
            task->submited_count=0;
            task->listName=fullarg;

            time_t now=time(NULL);
            for(int i=0;i<m.Count();i++)
            {
              newsms.setDestinationAddress(m[i]);
              int dlgId=GetNextDialogId();
              SmscCommand snd=SmscCommand::makeSumbmitSm(newsms,dlgId);
              putIncomingCommand(snd,now+i/sendSpeed);
              task->list.resize(task->count+1);
              task->list[task->count].addr = m[i];
              task->list[task->count].dialogId = dlgId;
              task->list[task->count].responsed = false;
              TPAIR p(task.get(),i);
              task_map.insert( pair<unsigned,TPAIR>(task->list[i].dialogId,p) );
              task->count++;
            }
            task->expirationTime = now+m.Count()/sendSpeed+WAIT_SUBMISSION;
            task_sheduler.push_back(task.release());
          }catch(IllegalSubmitterException& e)
          {
            reason="access_denied";
          }
        }
#ifdef DLSMSADMIN
        else if(cmdstr=="addprincipal")
        {
          Principal p(fullarg.c_str(),10,10);
          admin->addPrincipal(p);
        }
#endif //DLSMSADMIN

      }catch(smsc::core::buffers::FileException& e)
      {
        reason="database_error";
      }catch(ListNotExistsException& e)
      {
        reason="list_not_found";
      }catch(exception& e)
      {
        __warning2__("distrlist exception:'%s'",e.what());
        reason="system_error";
      }catch(...)
      {
        reason="unknown";
      }

      if(smeMode && tmpl.length())
      {
        tmpl.insert(2,".sme");
      }

      if(reason.length())
      {
        if(smeMode)
        {
          reason="dl.sme.reason."+reason;
        }else
        {
          reason="dl.reason."+reason;
        }
        smsc::profiler::Profile p=profiler->lookup(sms.getOriginatingAddress());
        try{
          std::string reasontxt=smsc::resourcemanager::ResourceManager::getInstance()->getString(p.locale,reason);
          if(reasontxt.length())
          {
            reason=reasontxt;
          }
        }catch(...)
        {
          __warning2__("failed to get resource %s for locale %s",reason.c_str(),p.locale.c_str());
        }
      }

      SMS ans;
      ans.setOriginatingAddress(
        smeMode?originatingAddressForSme.c_str():
        originatingAddress.length()?originatingAddress.c_str():sms.getDestinationAddress());
      ans.setDestinationAddress(sms.getOriginatingAddress());
      char msc[]="";
      char imsi[]="";
      ans.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
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
        if(tmpl.length())
        {
          answer="#template="+tmpl+"#{reason}=\""+reason+"\" {arg1}=\""+arg+"\" {arg2}=\""+arg2+"\"";
        }
      }
      if(answer.length())
      {
        smsc::util::fillSms(&ans,answer.c_str(),(int)answer.length(),CONV_ENCODING_CP1251,DataCoding::UCS2|DataCoding::LATIN1);
        SmscCommand cmdAnswer=SmscCommand::makeSumbmitSm(ans,GetNextDialogId());
        putIncomingCommand(cmdAnswer,0);
      }
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

void DistrListProcess::putIncomingCommand(const SmscCommand& cmd,time_t t)
{
  mon.Lock();
  inQueue.insert(InQueue::value_type(t,cmd));
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
  if ( multi->number_of_dests == 0 )
  {
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
  task->taskType=ttMulti;
  __trace2__(":DPL: numer of dests %d",multi->number_of_dests);
  for ( unsigned i=0; i < multi->number_of_dests; ++i )
  {
    if ( task->count >= MAX_COUNT )
    {
      throw BIG_MULTI();
    }
    if ( multi->dests[i].dest_flag )
    {
      __trace2__(":DPL: distrib list %s",multi->dests[i].value.c_str());
      Array<Address> addresses;
      try{
        addresses = admin->members(multi->dests[i].value,multi->msg.getOriginatingAddress());
      }catch(exception& e){
        __trace2__(":DPL: <exception> : %s",e.what());
        continue;
      }
      unsigned count = addresses.Count();
      if ( count+task->count > MAX_COUNT )
      {
        __trace2__(":DPL: members count(%d)+task->count(%d) great then MAX_COUNT for task",count,task->count);
        throw BIG_MULTI();
      }
      for ( unsigned j = 0; j < count; ++j )
      {
        __trace2__(":DPL:MEMBER %d.%d.%s",addresses[j].type,addresses[j].plan,addresses[j].value);
        task->list.resize(task->count+1);
        task->list[task->count].addr = addresses[j];
        task->list[task->count].dialogId = 0;
        task->list[task->count].responsed = false;
        task->list[task->count].errcode = Status::CNTSUBDL;
        ++task->count;
      }
    }
    else
    {
      __trace2__(":DPL: distrib addr %d.%d.%s",multi->dests[i].ton,multi->dests[i].npi,multi->dests[i].value.c_str());
      if( multi->dests[i].value.length() == 0 )
      {
        throw INV_DSTADDR();
      }
      task->list.resize(task->count+1);
      task->list[task->count].addr = Address(
        (int)multi->dests[i].value.length(),
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
  if ( task->count != 0 )
  {
    time_t now=time(NULL);
    for ( unsigned i=0; i<task->count; ++i )
    {
      task->list[i].dialogId = GetNextDialogId();
      SMS& msg = multi->msg;
      msg.setDestinationAddress(task->list[i].addr);
      TPAIR p(task.get(),i);
      task_map.insert( pair<unsigned,TPAIR>(task->list[i].dialogId,p) );
      __trace2__(":DPL:DEST %d.%d.%s",task->list[i].addr.type,task->list[i].addr.plan,task->list[i].addr.value);
      putIncomingCommand(SmscCommand::makeSumbmitSm(msg,task->list[i].dialogId),now+i/sendSpeed);
      __trace2__(":DPL: task %d of (0x%x:%d) has been scheduled for submit",
        task->list[i].dialogId,
        task.get(),
        task->count);
    }
    task->expirationTime = now+task->count/sendSpeed+WAIT_SUBMISSION;
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
  if ( it != task_map.end() )
  {
    TPAIR taskpair = it->second;
    if ( taskpair.second >= taskpair.first->count )
    {
      __warning__(":DPL: out of adresses range");
      //     !!!
    }else
    {
      ListTask* task = taskpair.first;
      task->list[taskpair.second].responsed = true;//cmd->get_resp()->get_status() == 0;
      task->list[taskpair.second].errcode = cmd->get_resp()->get_status();
      task_map.erase(it);
      task->submited_count++;
      __trace2__(":DPL: task %d of (0x%x:%d:%d) has been submited",cmd->get_dialogId(),task,task->count,task->submited_count);
      if ( task->submited_count == task->count )
      {
        __trace__(":DPL: send submit responce");
        if(task->taskType==ttMulti)
        {
          SendSubmitResp(task);
        }else
        {
          SendDLAnswer(task);
        }
      }
    }
  }else
  {
    __warning2__(":DPL: has no task with dialogid %d",cmd->get_dialogId());
  }
}

void DistrListProcess::SendDLAnswer(ListTask* task)
{
  SMS& sms=*task->cmd->get_sms();

  SMS ans;
  ans.setOriginatingAddress(originatingAddress.length()?originatingAddress.c_str():sms.getDestinationAddress());
  ans.setDestinationAddress(sms.getOriginatingAddress());
  char msc[]="";
  char imsi[]="";
  ans.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
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

  int okCnt=0;
  for(int i=0;i<task->count;i++)
  {
    if(task->list[i].responsed && task->list[i].errcode==0)
    {
      okCnt++;
    }
  }

  char buf1[32];
  sprintf(buf1,"%d",okCnt);
  char buf2[32];
  sprintf(buf2,"%d",task->count);

  string answer="#template=dl.sendok#{arg1}=\""+task->listName+"\" {arg2}="+buf1+" {arg3}="+buf2;
  smsc::util::fillSms(&ans,answer.c_str(),(int)answer.length(),CONV_ENCODING_CP1251,DataCoding::UCS2|DataCoding::LATIN1);
  SmscCommand cmdAnswer=SmscCommand::makeSumbmitSm(ans,GetNextDialogId());
  putIncomingCommand(cmdAnswer,0);

  LISTTYPE::iterator it = find(task_sheduler.begin(),task_sheduler.end(),task);
  if ( it != task_sheduler.end() )
  {
    delete *it;
    task_sheduler.erase(it);
  }
}


void DistrListProcess::SendSubmitResp(ListTask* task) //
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
        if (!task->list[i].responsed) task_map.erase(task->list[i].dialogId); //
        ue[uno].addr = task->list[i].addr;
        ue[uno].errcode = task->list[i].errcode; //0; // !!!!! must be fixed
        ++uno;
      }
    }
    __trace2__(":DPL: unsoccess %d",uno);
    cmd->get_MultiResp()->set_unsuccessCount(uno);
  //}
  SmeProxy* srcproxy =  task->cmd.getProxy();
  srcproxy->putCommand(cmd);
  LISTTYPE::iterator it = find(task_sheduler.begin(),task_sheduler.end(),task);
  if ( it != task_sheduler.end() )
  {
    delete *it;
    task_sheduler.erase(it);
  }
  //putIncomingCommand(cmd);
}

void DistrListProcess::CheckTimeouts()
{
  time_t curTime = time(0);
  while ( !task_sheduler.empty() && (task_sheduler.front()->expirationTime < curTime) )
  {
    //__trace2__(":DPL: T:%s task 0x%x(T:%s) was time out",
    //  ctime(&curTime),task_sheduler.front(),ctime(&task_sheduler.front()->startTime));
    ListTask* task=task_sheduler.front();
    for(int i=0;i<task->count;i++)
    {
      MAPTYPE::iterator it=task_map.find(task->list[i].dialogId);
      if(it!=task_map.end())
      {
        task_map.erase(it);
      }
    }
    if(task->taskType==ttMulti)
    {
      SendSubmitResp(task); //
    }else
    {
      SendDLAnswer(task);
    }
  }
}

}} // namespaces
