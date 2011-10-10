
#if !defined __header_DistrListProcess_h__
#define __header_DistrListProcess_h__

#include "../sms/sms.h"
#include "../smeman/smeman.h"
#include "../core/synchronization/EventMonitor.hpp"
#include "../core/threads/ThreadedTask.hpp"
#include "../util/debug.h"
#include "../core/buffers/Array.hpp"
#include "DistrListAdmin.h"
#include <map>
#include <list>
#include <vector>
#include <time.h>
#include "profiler/profiler-types.hpp"
#include "alias/AliasMan.hpp"

namespace smsc{
namespace distrlist{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace std;

class DistrListProcess : public ThreadedTask, public SmeProxy
{
protected:
  enum {MAX_COUNT=256};
  struct LISTELEMENT
  {
    unsigned dialogId;
    Address addr;           // ����� ����������
    bool responsed;
    unsigned errcode;
  };
  struct ListTask;
  friend struct DistrListProcess::ListTask;
  enum TaskType{ttMulti,ttDistrList};
  struct ListTask
  {
    TaskType taskType;//0 - submit multi, 1 - distr list
    time_t expirationTime;
    std::string listName;
    unsigned count;           // ���������� ��������� � ������
    unsigned submited_count;  // ���������� ��������� �� ������� ������ �����
    std::vector<LISTELEMENT> list;
    SmscCommand cmd;          // �������� �������
  };
  typedef pair<ListTask*,unsigned> TPAIR;
  typedef std::map<unsigned,TPAIR> MAPTYPE;
  typedef list<ListTask*> LISTTYPE;
  MAPTYPE task_map;       // ������� ������ <-> {������,������ ��������}
  LISTTYPE task_sheduler; // ������ ����� ������������ �� ���������
  DistrListAdmin* admin;
  mutable EventMonitor mon;
  Array<SmscCommand> outQueue;
  typedef std::multimap<time_t,SmscCommand> InQueue;
  InQueue inQueue;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  int seq;
  SmeRegistrar* smereg;


  smsc::profiler::ProfilerInterface* profiler;
  smsc::alias::AliasManager* aliaser;

  Address dealias(const std::string& addr)
  {
    Address rv;
    if(aliaser->AliasToAddress(addr.c_str(),rv))
    {
      return rv;
    }
    return addr.c_str();
  }

public:
  DistrListProcess(DistrListAdmin* admin,SmeRegistrar* sm);
  virtual ~DistrListProcess();
  virtual void close();
  /// ������ exception ���� ��� ��������� �����
  /// � ����� ������� ��� �� ����� �� ����������� ��������
  virtual void putCommand(const SmscCommand& command);
  virtual bool getCommand(SmscCommand& cmd);
  virtual SmeProxyState getState() const;
  virtual void init();
  virtual SmeProxyPriority getPriority()const {return SmeProxyPriorityDefault;};
  virtual void setPriority(SmeProxyPriority){};
  virtual bool hasInput() const;
  virtual void attachMonitor(ProxyMonitor* monitor); // for detach monitor call with NULL
  virtual bool attached(); // check what proxy has attached monitor (proxy is attached to dispatcher)
  virtual uint32_t getNextSequenceNumber();
  virtual uint32_t getUniqueId() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
  virtual uint32_t getIndex() const {__unreachable__("must be implemented in proxy or wrapper");return 0;}
  virtual unsigned long getPreferredTimeout() { return 8; }
  virtual const std::string& getSourceAddressRange(){return nullstr;};
  virtual const char * getSystemId() const;
// ThreadedTask
  virtual int Execute();
  virtual const char* taskName();

  void assignProfiler(smsc::profiler::ProfilerInterface* argProfiler)
  {
    profiler=argProfiler;
  }
  void assignAliaser(smsc::alias::AliasManager* argAliaser)
  {
    aliaser=argAliaser;
  }


  string serviceType;
  int protocolId;

  std::string originatingAddress;
  std::string originatingAddressForSme;

  bool autoCreatePrincipal;
  int defaultMaxLists;
  int defaultMaxElements;
  int sendSpeed;

protected:
  void putIncomingCommand(const SmscCommand& cmd,time_t t);
  SmscCommand getOutgoingCommand();
  bool hasOutput();
  void waitFor(unsigned timeout);
  void SubmitMulti(SmscCommand& cmd);
  void SubmitResp(SmscCommand& cmd);
  void SendSubmitResp(ListTask* task);
  void SendDLAnswer(ListTask* task);
  void CheckTimeouts();
  void Submit();
  //Smsc *smsc;
};

}} // namespaces


#endif // __header_DistrListProcess_h__
