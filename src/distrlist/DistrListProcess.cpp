// $Id$

#include "DistrListProcess.h"
#include "../system/status.h"
#define DLP_TIMEOUT 1000
#define WAIT_SUBMISSION (1000*8)

namespace smsc{
namespace distrlist{

using namespace smsc::system;

static uint32_t GetNextDialogId()
{
  uint32_t id = 0;
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

/// ������ exception ���� ��� ��������� �����
/// � ����� ������� ��� �� ����� �� ����������� ��������
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
        putIncomingCommand(cmdR);
      }catch(BIG_MULTI&){
        __trace2__(":DPL: <exception> counts of member of multi great then MAX_COUNT for task");
        SmscCommand cmdR = SmscCommand::makeSubmitMultiResp(0,cmd->get_dialogId(),Status::CNTSUBDL);
        putIncomingCommand(cmdR);
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
    return;
  }
  auto_ptr<ListTask> task(new ListTask());
  task->count = 0;
  for ( unsigned i=0; i < multi->number_of_dests; ++i )
  {
    if ( task->count >= MAX_COUNT ) {
      throw BIG_MULTI();
    }
    if ( multi->dests[i].dest_flag ){
      Array<Address> addresses = admin->members(multi->dests[i].value,multi->msg.getOriginatingAddress());
      unsigned count = addresses.Count();
      if ( count+task->count > MAX_COUNT ) {
        __trace2__(":DPL: members count(%d)+task->count(%d) great then MAX_COUNT for task",
                      count,task->count);
        throw BIG_MULTI();
      }
      for ( unsigned i = 0; i < count; ++i )
      {
        task->list[task->count].addr = addresses[i];
        task->list[task->count].dialogId = 0;
        task->list[task->count].success = false;
        ++task->count;
      }
    }
    else{
     task->list[task->count].addr = Address(
       multi->dests[i].value.length(),
       multi->dests[i].ton,
       multi->dests[i].npi,
       multi->dests[i].value.c_str());
     task->list[task->count].dialogId = 0;
     task->list[task->count].success = false;
      ++task->count;
    }
  }
  task->submited_count = 0;
  for ( unsigned i=0; i<task->count; ++i )
  {
    task->list[i].dialogId = GetNextDialogId();
    SMS msg = multi->msg;
    msg.setDestinationAddress(task->list[i].addr);
    TPAIR p(task.get(),i);
    task_map.insert( pair<unsigned,TPAIR>(task->list[i].dialogId,p) );
    putIncomingCommand(SmscCommand::makeSumbmitSm(multi->msg,task->list[i].dialogId));
    __trace2__(":DPL: task %d of (0x%x:%d) has been scheduled for submit",
      task->list[i].dialogId,
      task.get(),
      task->count);
  }
  task->startTime = time(0);
  task_sheduler.push_back(task.get());
  __trace2__(":DPL: task list 0x%x has been scheduled",task.get());
  task.release();
}

void DistrListProcess::SubmitResp(SmscCommand& cmd)
{
  /*map<pair<unsigned,pair<ListTask*,unsigned> > >*/
  MAPTYPE::iterator it = task_map.find(cmd->get_dialogId());
  if ( it != task_map.end() ) {
    TPAIR taskpair = it->second;
    if ( taskpair.second >= taskpair.first->count ) {
      // �� �������� ������� ������� !!!
    }else{
      ListTask* task = taskpair.first;
      task->list[taskpair.second].success = cmd->get_resp()->get_status() == 0;
      task_map.erase(task->list[taskpair.second].dialogId);
      task->submited_count++;
      __trace2__(":DPL: task %d of (0x%x:%d:%d) has been submited",
        cmd->get_dialogId(),
        task,
        task->count,
        task->submited_count);
      if ( task->submited_count == task->count ) SendSubmitResp(task);
    }
  }else{
    __trace2__(":DPL: has no task with dialogid %d",cmd->get_dialogId());
  }
}

void DistrListProcess::SendSubmitResp(ListTask* task) // ������� �� ������ � ����
{
  unsigned status = 0;
  if ( task->count != task->submited_count ) status = Status::CNTSUBDL; /// !!!!!! must be fixed
  SmscCommand cmd = SmscCommand::makeSubmitMultiResp(0,task->cmd->get_dialogId(),status);
  if ( status != 0 ) {
    UnsuccessElement* ue = cmd->get_MultiResp()->get_unsuccess();
    cmd->get_MultiResp()->set_unsuccessCount(task->count-task->submited_count);
    for ( unsigned i=0,j=0; i < task->count; ++i )
    {
      __require__ ( j  < 256 );
      if ( !task->list[i].success ) {
        task_map.erase(task->list[i].dialogId); // ��������� ��� �������
        ue[j].addr = task->list[i].addr;
        ue[j].errcode = 0; // !!!!! must be fixed
      }
    }
  }
  LISTTYPE::iterator it = find(task_sheduler.begin(),task_sheduler.end(),task);
  if ( it != task_sheduler.end() ) { task_sheduler.erase(it);  }
  putIncomingCommand(cmd);
}

void DistrListProcess::CheckTimeouts()
{
  time_t curTime = time(0)+WAIT_SUBMISSION;
  while ( !task_sheduler.empty() && (task_sheduler.front()->startTime < curTime) )
  {
    __trace2__(":DPL: T:%s task 0x%x(T:%s) was time out",
      ctime(&curTime),task_sheduler.front(),ctime(&task_sheduler.front()->startTime));
    SendSubmitResp(task_sheduler.front()); // ������� �� ������ � ����
  }
}

}} // namespaces
