static char const ident[] = "$Id$";

#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "sms/sms.h"
#include "logger/Logger.h"
#include <queue>

namespace smsc{namespace mtsmsme{namespace processor{
using smsc::sms::AddressValue;
using smsc::logger::Logger;

static Logger* logger = 0;
static TCO* coordinator =  0;                                                                                        
static string msc_digits;                                                                                            
static string vlr_digits;                                                                                            
// list of registration tasks
class SubscriberRegistrationTask;
class SubscriberRegistrationTaskComparator { public: bool operator()(SubscriberRegistrationTask*,SubscriberRegistrationTask*);};                                                                                        
static std::priority_queue<SubscriberRegistrationTask*,std::vector<SubscriberRegistrationTask*>,SubscriberRegistrationTaskComparator> tqueue;                                                      
// list of pending update location invokations
class UpdateLocationTask;                                                                       
static std::queue<UpdateLocationTask*> pqueue;

class SubscriberRegistrationTask {
  public:
    string imsi;
    string msisdn;
    string mgt;
    string msc;
    string vlr;
    int period;
    time_t deadline;
    SubscriberRegistrationTask(
                                 const string& _imsi,
                                 const string& _msisdn,
                                 const string& _mgt,
                                 const string& _msc,
                                 const string& _vlr,
                                 const time_t _period
                                ):imsi(_imsi), msisdn(_msisdn),mgt(_mgt),msc(_msc),vlr(_vlr),period(_period) {}
    void setDelay(int delay) { deadline = time(0) + delay; }
};
bool SubscriberRegistrationTaskComparator::operator()(SubscriberRegistrationTask* left, SubscriberRegistrationTask* right)
{
  return ((left->deadline) >= (right->deadline));
}
class UpdateLocationTask: public TsmComletionListener{
  private:
    int status;
  public:
    UpdateLocationTask(SubscriberRegistrationTask& _info):status(0),info(_info){}
    SubscriberRegistrationTask info;
    void changeStatus(int _status)                                                                                   
    {                                                                                                                
      status = _status;                                                                                              
      if (status == 1)                                                                                               
        smsc_log_debug(logger,                                                                                       
                 "COMPLETED UPDATELOCATION imsi=\'%s\'",info.imsi.c_str());                                  
    }
    void process(TCO* coordinator)
    {
      smsc_log_debug(logger,                                                                                             
                 "FAKE UPDATELOCATION imsi=\'%s\', msisdn=\'%s\', mgt=\'%s\' with period=%d seconds" 
                 " serving by msc=\'%s\', vlr=\'%s\'",                                                               
                 info.imsi.c_str(), info.msisdn.c_str(), info.mgt.c_str(), info.period,                                                     
                 msc_digits.c_str(), vlr_digits.c_str());
      //changeStatus(1);
      if (coordinator) 
      {
        TSM* tsm;
        tsm = coordinator->TC_BEGIN(info.imsi.c_str(), msc_digits.c_str(), vlr_digits.c_str(), info.mgt.c_str());
        if (tsm) tsm->BeginTransaction(this);
      }
    }
    bool isCompleted() { return (status == 1); }
    virtual void complete(int _status) { changeStatus(1); }
};

SubscriberRegistrator::SubscriberRegistrator(TCO* _coordinator)
{
  logger = Logger::getInstance("mt.sme.reg");
  coordinator = _coordinator;
}
void SubscriberRegistrator::configure(Address& _msc, Address& _vlr)
{
  AddressValue vlrnumber;
  AddressValue mscnumber;
  _msc.getValue(mscnumber); msc_digits = mscnumber;
  _vlr.getValue(vlrnumber); vlr_digits = vlrnumber;
}

/**
 * request to register specified info to HLR on periodical basis specified by period.                         
 * if 'period' parameter equals zero then register info only once                                             
 */
void SubscriberRegistrator::registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period)
{
  AddressValue imsi_digits;  imsi.getValue(imsi_digits); 
  AddressValue msisdn_digits; msisdn.getValue(msisdn_digits);
  AddressValue mgt_digits; mgt.getValue(mgt_digits);
  SubscriberRegistrationTask* info;
 
  info = new  SubscriberRegistrationTask(imsi_digits,msisdn_digits,mgt_digits,msc_digits,vlr_digits,period);
  info->setDelay(60); //additional delay on starting update location
  tqueue.push(info);                                                                                                 
  smsc_log_debug(logger,                                                                                             
                 "register update location task for imsi=\'%s\' with period=%d seconds",                             
                 info->imsi.c_str(),info->period);
  //smsc_log_debug(logger,
  //               "register request for UPDATELOCATION imsi=\'%s\', msisdn=\'%s\', mgt=\'%s\' with period=%d seconds"
  //               " serving by msc=\'%s\', vlr=\'%s\'",
  //               imsi_digits, msisdn_digits, mgt_digits, period,
  //               msc_digits.c_str(), vlr_digits.c_str());
  //smsc_log_debug(logger,"register update location task for imsi=\'%s\' with period=%d seconds",imsi_digits, period);
}
void SubscriberRegistrator::process()
{
  //for pending update location task check completion status
  //update subscriber database and block new registration invokation
  if (! pqueue.empty())
  {
    UpdateLocationTask* task = pqueue.front();
    if (task->isCompleted())
    {
      pqueue.pop();
      if (task->info.period)
      {
        int period = task->info.period;
        SubscriberRegistrationTask* info = new SubscriberRegistrationTask(task->info);
        info->setDelay(task->info.period);
        tqueue.push(info);
      }
      delete(task);
    }
    return; //processing queue is not empty
  }
  if (! tqueue.empty())
  {
    time_t now; time(&now);
    SubscriberRegistrationTask* request = tqueue.top();
    if (now > request->deadline)
    {
      tqueue.pop();
      UpdateLocationTask* task = new UpdateLocationTask(*request);
      delete(request);
      pqueue.push(task);
      task->process(coordinator);
    }
  }
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
