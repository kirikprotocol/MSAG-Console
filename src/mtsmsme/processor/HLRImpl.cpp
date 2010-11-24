static char const ident[] = "$Id$";

#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/UpdateLocation.hpp"
#include "sms/sms.h"
#include "logger/Logger.h"
#include "mtsmsme/processor/util.hpp"
#include "TCO.hpp"
#include <queue>

namespace smsc{namespace mtsmsme{namespace processor{
using smsc::sms::AddressValue;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::comp::UpdateLocationReq;
using smsc::mtsmsme::processor::TCO;

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
      switch (status)
      {
      case 1:
        smsc_log_debug(logger,
            "COMPLETED UPDATELOCATION imsi=\'%s\'",info.imsi.c_str());
        break;
      case 1143:
        smsc_log_debug(logger,
            "NO RESP FROM HLR UPDATELOCATION imsi=\'%s\'",info.imsi.c_str());
        break;
      default:
        smsc_log_debug(logger,
            "UNKNOWN STATUS UPDATELOCATION imsi=\'%s\'",info.imsi.c_str());
        break;
      }
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
        AC appcntx = net_loc_upd_v2;
        //tsm = coordinator->TC_BEGIN(info.imsi.c_str(), msc_digits.c_str(), vlr_digits.c_str(), info.mgt.c_str());
        tsm = coordinator->TC_BEGIN(appcntx);
        if (tsm)
        {
          tsm->setCompletionListener(this);

          UpdateLocationReq msg;
          msg.setParameters(info.imsi, msc_digits,vlr_digits);
          tsm->TInvokeReq( 1 /* invokeId */, 2 /* updateLocation operation */, msg);

          uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
          cllen = packSCCPAddress(cl, 1 /* E.164 */, vlr_digits.c_str() /* VLR E.164 */, 7 /* VLR SSN */);
          cdlen = packSCCPAddress(cd, 7 /* E.214 */, info.mgt.c_str()   /* MS  E.214 */, 6 /* HLR SSN */);
          tsm->TBeginReq(cdlen, cd, cllen, cl);
        }
      }
    }
    bool isCompleted() { return (status != 0); }
    virtual void complete(int _status) { changeStatus(_status); }
    virtual void complete(Message& msg) { changeStatus(1); }
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
int SubscriberRegistrator::update(Address& imsi, Address& msisdn, Address& mgt)
{
  smsc_log_debug(logger,
                 "register MSISDN in INTRERNAL HLR "
                 "imsi=\'%s\', msisdn=\'%s\', mgt=\'%s\'"
                 " serving by msc=\'%s\', vlr=\'%s\'",
                 imsi.toString().c_str(), msisdn.toString().c_str(),
                 mgt.toString().c_str(), msc_digits.c_str(), vlr_digits.c_str());
  hlr.insert(pair<const string,string>(msisdn.value,imsi.value));
  return 0;
}
typedef map<string,string>::iterator hlriter;
bool SubscriberRegistrator::lookup(Address& msisdn, Address& imsi,Address& msc)
{
  hlriter pos = hlr.find(msisdn.value);
  if ( pos != hlr.end())
  {
    imsi.setValue(pos->second.size(),pos->second.c_str());
    msc.setValue(msc_digits.size(),msc_digits.c_str());
    smsc_log_debug(logger,
                   "hlr::lookup for %s succeded with %s locates at %s",
                   msisdn.toString().c_str(),
                   imsi.toString().c_str(),
                   msc.toString().c_str());
    return true;
  }
  smsc_log_debug(logger,"hlr::lookup for %s failed",msisdn.toString().c_str());
  return false;
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
