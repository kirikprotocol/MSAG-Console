#include <string>
#include "mtsmsme/sccp/SccpProcessor.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "sms/sms.h"

using smsc::mtsmsme::processor::SccpProcessor;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::core::threads::Thread;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using smsc::sms::Address;
using std::string;

static Logger *logger = 0;
class EmptySubscriberRegistrator: public SubscriberRegistrator {
  public:
    EmptySubscriberRegistrator(TCO* _tco) : SubscriberRegistrator(_tco) {}
    virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period) {}
    virtual int  update(Address& imsi, Address& msisdn, Address& mgt) {return 1;}
    virtual bool lookup(Address& msisdn, Address& imsi, Address& msc) {return false;}
};
class GopotaListener: public SccpProcessor, public Thread {
  public:
    GopotaListener(TCO* _tco, SubscriberRegistrator* _reg) : SccpProcessor(_tco, _reg){}
    virtual int Execute()
    {
      int result;
      result = Run();
      smsc_log_error(logger,"SccpListener exit with code: %d", result);
      return result;
    }
};

int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("sri4smreq");
  try
  {
    smsc_log_info(logger, "Send Routing Info For SM generator");
    TCO mtsms(10);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    GopotaListener listener(&mtsms,&fakeHLR);
    listener.configure(43,191,Address(".1.1.791398699812"),
                               Address(".1.1.791398699813"),
                               Address(".1.1.791398699813"));
    listener.Start();
    int8_t invoke_id = 0;
    int count = 0;
    bool pri;
    sleep(10);
    while(true)
    {
      /* SMSC = 79139860004, MSISDN=79139859489 */
      if (++count % 2) pri = true; else pri = false;
      string ms("79139859489"); // mobile station MSISDN
      string sca("791398699812"); // service center address
      uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
      cllen = packSCCPAddress(cl, 1 /* E.164 */, sca.c_str() /* SMSC E.164 */, 8 /* SMSC SSN */);
      cdlen = packSCCPAddress(cd, 1 /* E.164 */, ms.c_str() /* MS   E.164 */, 6 /* MS   SSN */);
      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
      if (tsm)
      {
        SendRoutingInfoForSMReq* inv = new SendRoutingInfoForSMReq(ms, pri, sca);
        tsm->TInvokeReq(invoke_id++, 45, *inv);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
      }
      sleep(10);
    }
    listener.Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, "catched unexpected exception [%s]", ex.what());
  }
  return 0;
}
