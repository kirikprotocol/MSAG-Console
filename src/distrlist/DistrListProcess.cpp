// $Id$

#include "DistrListProcess.h"
#include "../system/status.h"
#define DLP_TIMEOUT 1000
#define WAIT_SUBMISSION (8)

namespace smsc{
namespace distrlist{

using namespace smsc::system;

static uint32_t GetNextDialogId()
{
  static uint32_t id = 0;
  return ++id;
}

struct BIG_MULTI{};

DistrListProcess::DistrListProcess(DistrListAdmin* admin) :
  admin(admin),
  managerMonitor(0),
  seq(1)
{}

DistrListProcess::~DistrListProcess() {}
void DistrListProcess::close()  {}

/// кидает exception если был достигнут лимит
/// и длина очереди еще не упала до допустимого значения
void DistrListProcess::putCommand(const SmscCommand& cmd)
{
  MutexGuard g(mon);
  mon.notify();
  outQueue.Push(cmd);
}

SmscCommand DistrListProcess::getCommand()
{
  MutexGuard g(mon);
  SmscCommand cmd;
  inQueue.Shift(cmd);
  return cmd;
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
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.distrlist.process");
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
        __trace2__(":DPL: <exception> %s",e.what());
        SmscCommand cmdR = SmscCommand::makeSubmitMultiResp(0,cmd->get_dialogId(),Status::CNTSUBDL);
        cmd.getProxy()->putCommand(cmdR);
      }catch(BIG_MULTI&){
        __trace2__(":DPL: <exception> counts of member of multi great then MAX_COUNT for task");
        SmscCommand cmdR = SmscCommand::makeSubmitMultiResp(0,cmd->get_dialogId(),Status::CNTSUBDL);
        cmd.getProxy()->putCommand(cmdR);
      }
    }
    else if ( cmd->get_commandId() == SUBMIT )
    {
      // NOTHING
      __warning2__(":DLP: unknown SUBMIT (isn't implemented yet)");
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
    __trace2__(":DPL: sending responce immediate");
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
      __warning2__(":DPL: out of adresses range");
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
        __trace2__(":DPL: send submit responce");
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
