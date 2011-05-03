static char const ident[] = "$Id$";
#include "util/Exception.hpp"
#include "mtsmsme/processor/Processor.h"
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

using smsc::util::Exception;
using smsc::mtsmsme::processor::RequestProcessor;
using smsc::mtsmsme::processor::RequestProcessorFactory;
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

static Logger *logger = 0;
class EmptyRequestSender: public RequestSender {
  virtual bool send(Request* request)
  {
//    using namespace smsc::sms;
//    const char* prop = 0;
//    unsigned int len = 0;
//    prop = request->sms.getBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,&len);
//    smsc_log_info(logger,
//                           "SMS[%d]={%s}",len,dump(len,(unsigned char *)prop).c_str());
//sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,(char*)b.get(),udh_len+1+symbols);
    request->setSendResult(0); return true;
  }
};
static char etalon[] = "791398600044xxxx";
static char etalon_imsi[] = "25001389999xxxx";
static char etalon_msc[] = "791398600042";
static char sender_msc[] = "791398600045";
static char border_msisdn[] = "7913986000440099";
class EmptySubscriberRegistrator: public SubscriberRegistrator {
  public:
    EmptySubscriberRegistrator(TCO* _tco) : SubscriberRegistrator(_tco) {}
    virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period) {}
    virtual int  update(Address& imsi, Address& msisdn, Address& mgt) {return 1;}
    virtual bool lookup(Address& msisdn, Address& imsi, Address& msc)
    {
      //msisdn must be 791398699814xxx, so imsi will be 250013899999xxx
      if (strlen(msisdn.value) != strlen(etalon)) return false;
      if (strncmp(msisdn.value,etalon,strlen(etalon)-4) == 0)
      {
        strncpy(etalon_imsi+strlen(etalon_imsi)-4,msisdn.value+strlen(msisdn.value)-4,4);
        imsi.setValue((uint8_t)strlen(etalon_imsi),etalon_imsi);
        if (strncmp(msisdn.value,border_msisdn,strlen(etalon)) <= 0 )
          msc.setValue((uint8_t) strlen(sender_msc), sender_msc);
        else
          msc.setValue((uint8_t) strlen(etalon_msc), etalon_msc);
        return true;
      }
      return false;
    }
};
class GopotaListener: public Thread {
  private:
    RequestProcessor* requestProcessor;
  public:
    GopotaListener(TCO* _tco, SubscriberRegistrator* _reg)
    {
      RequestProcessorFactory* factory = 0;
      factory = RequestProcessorFactory::getInstance();
      if (!factory)
        throw Exception("RequestProcessorFactory is undefined");

      requestProcessor = factory->createRequestProcessor(_tco, _reg);
      if (!requestProcessor)
        throw Exception("RequestProcessor is undefined");
    }
    virtual int Execute()
    {
      int result;
      result = requestProcessor->Run();
      smsc_log_error(logger,"SuaListener exit with code: %d", result);
      return result;
    }
    void Stop()
    {
      requestProcessor->Stop();
    }
    void configure(int user_id, int ssn, Address& msc, Address& vlr, Address& hlr)
    {
      requestProcessor->configure(user_id,ssn,msc,vlr,hlr);
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
    listener.configure(43,191,Address(".1.1.791398600042"),
                               Address(".1.1.791398600043"),
                               Address(".1.1.791398600044"));
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
