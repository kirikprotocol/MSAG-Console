
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
#include <time.h>

namespace smsc{
namespace distrlist{

using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace std;

class DistrListProcess : public ThreadedTask, public SmeProxy{
  enum {MAX_COUNT=256};
  struct LISTELEMENT{
    unsigned dialogId;
    Address addr;           // адрес получателя
    bool success;           // удачность завершения посылки
  };
  struct ListTask{
    time_t startTime;
    unsigned count;           // количество елементов в задаче
    unsigned submited_count;  // количество элементов на которые прешол ответ
    LISTELEMENT list[MAX_COUNT];
    SmscCommand cmd;          // исходная команда
  };
  typedef pair<ListTask*,unsigned> TPAIR;
  typedef map<unsigned,TPAIR> MAPTYPE;
  typedef list<ListTask*> LISTTYPE;
  MAPTYPE task_map;       // маппинг диалог <-> {задача,индекс элемента}
  LISTTYPE task_sheduler; // список задач упорядоченый по таймаутам
  DistrListAdmin* admin;
  mutable EventMonitor mon;
  Array<SmscCommand> outQueue;
  Array<SmscCommand> inQueue;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  int seq;
public:
  DistrListProcess(DistrListAdmin* admin);
  virtual ~DistrListProcess();
  virtual void close();
  /// кидает exception если был достигнут лимит
  /// и длина очереди еще не упала до допустимого значения
  virtual void putCommand(const SmscCommand& command);
  virtual SmscCommand getCommand();
  virtual SmeProxyState getState() const;
  virtual void init();
  virtual SmeProxyPriority getPriority() {return SmeProxyPriorityDefault;};
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
protected:
  void putIncomingCommand(const SmscCommand& cmd);
  SmscCommand getOutgoingCommand();
  bool hasOutput();
  void waitFor(unsigned timeout);
  void SubmitMulti(SmscCommand& cmd);
  void SubmitResp(SmscCommand& cmd);
  void SendSubmitResp(ListTask* task);
  void CheckTimeouts();
  //Smsc *smsc;
};

}} // namespaces


#endif // __header_DistrListProcess_h__
