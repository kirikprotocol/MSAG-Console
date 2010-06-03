static char const ident[] = "$Id";
#include "mtsmsme/sua/SuaProcessor.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include "sms/sms.h"
#include <string>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

#define TRANS_TYPE 0
#define NUM_PLAN 0x10
#define ENC_SCHEME 0x01
#define NATURE_OF_ADDR 0x04

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
using smsc::sms::Address;
using std::string;

static char msca[] = "791398699924"; // MSC address
static char vlra[] = "79139860004"; //VLR address
static char hlra[] = "79139860004"; //HLR address

static Logger *logger = 0;
class EmptyRequestSender: public RequestSender {
  virtual bool send(Request* request)
  {
    Request& req = *request;
    req.setSendResult(0);
    //req.dstmsc;
    return true;
  }
};
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
int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("mschlr");

  try
  {
    smsc_log_info(logger, "MSC and HLR emulator");
    using smsc::core::threads::Thread;
    EmptyRequestSender fakeSender;
    TCO mtsms(10000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms,&fakeHLR);
    DialogueStat stat;
    listener.configure(43, 191, Address((uint8_t)strlen(msca), 1, 1, msca),
        Address((uint8_t)strlen(vlra), 1, 1, vlra),
        Address((uint8_t)strlen(hlra), 1, 1, hlra));
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
