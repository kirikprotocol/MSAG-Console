static char const ident[] = "$Id$";
#include "core/synchronization/EventMonitor.hpp"
#include "mtsmsme/sua/SuaProcessor.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"
#include "sms/sms.h"
#include <string>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

#define TRANS_TYPE 0
#define NUM_PLAN 0x10
#define ENC_SCHEME 0x01
#define NATURE_OF_ADDR 0x04

using smsc::core::synchronization::EventMonitor;
using smsc::mtsmsme::processor::SuaProcessor;
using smsc::mtsmsme::processor::RequestSender;
using smsc::mtsmsme::processor::Request;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::core::threads::Thread;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::TSMSTAT;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
using smsc::sms::Address;
using std::string;

static char msca[20] = "791398699874"; // MSC address
static char vlra[20] = "79139860004"; //VLR address
static char hlra[20] = "79139860004"; //HLR address
static int  ssn = 191; //SSN
static int  userid = 43; //common part user id
static int  delay = 8; // MT SMS time processing

static Logger *logger = 0;
class EmptySubscriberRegistrator: public SubscriberRegistrator {
  public:
    EmptySubscriberRegistrator(TCO* _tco) : SubscriberRegistrator(_tco) {}
    virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period) {}
    virtual int  update(Address& imsi, Address& msisdn, Address& mgt) {return 1;}
    virtual bool lookup(Address& msisdn, Address& imsi, Address& msc) {return false;}
};
class GopotaListener: public SuaProcessor, public Thread {
  public:
    GopotaListener(TCO* _tco, SubscriberRegistrator* _reg) : SuaProcessor(_tco,_reg) {}
    virtual int Execute()
    {
      int result;
      result = Run();
      smsc_log_error(logger,"SccpListener exit with code: %d", result);
      return result;
    }
};
class DialogueStat: public Thread {
  private:
    TSMSTAT stat;
  public:
    virtual int Execute()
    {
      while (true)
      {
        struct timespec delay = { 0, 995000000 }; // 955ms
        TSMSTAT statnow;
        TSM::getCounters(statnow);
        smsc_log_info(logger,"dlg cre/del/cnt = %d/%d/%d",
            statnow.objcreated - stat.objcreated,
            statnow.objdeleted - stat.objdeleted,
            statnow.objcount);
        stat = statnow;
        nanosleep(&delay,0);
      }
      return 0;
    }
  };
class mtresptimer {
  public:
    //time_t deadline; Request* req;
    struct timeval deadline; Request* req;
    mtresptimer(int delay, Request* _req):
      //deadline(time(0)+delay),req(_req){}
      req(_req)
    {
      gettimeofday(&deadline,NULL);
      deadline.tv_sec += delay;
    }
};
class DelayedRequestSender: public RequestSender, public Thread
{
  private:
    int delay;
    Logger* logger;
    Mutex lock;
    EventMonitor mon;
    std::list<mtresptimer> queue;
  public:
    DelayedRequestSender(int _delay, Logger* _logger) : delay(_delay),
      logger(_logger)
    {
    }
    virtual bool send(Request* req)
    {
      if (delay == 0) { req->setSendResult(0); return true; }
      //if delay > 0 place request to queue and ACK
      {
        MutexGuard g(lock);
        queue.push_back(mtresptimer(delay, req));
      }
      return true;
    }
    virtual int Execute()
    {
      while (1)
      {
        MutexGuard guard(mon);
        //time_t now; time(&now);
        struct timeval now;
        gettimeofday(&now,NULL);
        while (!queue.empty())
        {
          mtresptimer timer = queue.front();
          //if (now < timer.deadline)
          if ( timercmp(&now,&timer.deadline,<))
            break;
          {
            MutexGuard g(lock);
            queue.pop_front();
          }
          timer.req->setSendResult(0);
        }
        mon.wait(100);
      }
      return 0;
    }
};
class ToolConfig {
  private:
    int cfgdelay;
    Logger* logger;
  public:
    ToolConfig(Logger* _logger):logger(_logger){}
    void read(Manager& manager)
    {
      if (!manager.findSection("msc-mtsm"))
        throw ConfigException("\'msc-mtsm\' section is missed");

      ConfigView view(manager, "msc-mtsm");

      try { cfgdelay = view.getInt("delay");
      } catch (ConfigException& exc) {
        throw ConfigException("\'delay\' is unknown or missing");
      }
      delay = cfgdelay;
    }
};
class SccpConfig {
  private:
    Logger* logger;
  public:
    SccpConfig(Logger* _logger):logger(_logger){}
    void read(Manager& manager)
    {
      char* tmp_str;
      Address tmp_addr;

      if (!manager.findSection("sccp"))
        throw ConfigException("\'sccp\' section is missed");

      ConfigView sccpConfig(manager, "sccp");

      try { userid = sccpConfig.getInt("user_id");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_id\' is unknown or missing");
      }

      try { ssn = sccpConfig.getInt("user_ssn");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_ssn\' is unknown or missing");
      }

      try {
        tmp_str = sccpConfig.getString("msc_gt");
        tmp_addr = Address(tmp_str);
        tmp_addr.getValue(msca);
      } catch (ConfigException& exc) {
        throw ConfigException("\'msc_gt\' is unknown or missing");
      }

      try {
        tmp_str = sccpConfig.getString("vlr_gt");
        tmp_addr = Address(tmp_str);
        tmp_addr.getValue(vlra);
      } catch (ConfigException& exc) {
        throw ConfigException("\'vlr_gt\' is unknown or missing");
      }

      try {
        tmp_str = sccpConfig.getString("hlr_gt");
        tmp_addr = Address(tmp_str);
        tmp_addr.getValue(hlra);
      } catch (ConfigException& exc) {
        throw ConfigException("\'hlr_gt\' is unknown or missing");
      }
    }
};
int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("mschlr");

  try
  {
    smsc_log_info(logger, "MT SMS receiver");
    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();
    ToolConfig config(logger);
    config.read(manager);

    using smsc::core::threads::Thread;
    DelayedRequestSender fakeSender(delay,logger); // or EmptyRequestSender
    TCO mtsms(100000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms,&fakeHLR);
    DialogueStat stat;
    listener.configure(ssn, userid, Address((uint8_t)strlen(msca), 1, 1, msca),
        Address((uint8_t)strlen(vlra), 1, 1, vlra),
        Address((uint8_t)strlen(hlra), 1, 1, hlra));
    fakeSender.Start();
    listener.Start();
    stat.Start();
    int8_t invoke_id = 0;
    int count = 0;
    bool pri;
    sleep(10);
    while(true)
    {
      sleep(60);
    }
    listener.Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " catched unexpected exception [%s]", ex.what());
  }
  return 0;
}
