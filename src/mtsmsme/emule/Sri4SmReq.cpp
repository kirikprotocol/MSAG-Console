#include <string>
#include "core/threads/Thread.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/sua/SuaProcessor.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "sms/sms.h"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

#define TRANS_TYPE 0
#define NUM_PLAN 0x10
#define ENC_SCHEME 0x01
#define NATURE_OF_ADDR 0x04

using smsc::mtsmsme::processor::SuaProcessor;
using smsc::core::threads::Thread;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::sms::Address;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using std::string;


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
      smsc_log_error(logger,"SuaListener exit with code: %d", result);
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
    GopotaListener listener(&mtsms, &fakeHLR);

    listener->configure(43,191,Address(".1.1.791398699812"),
                               Address(".1.1.791398699813"),
                               Address(".1.1.791398699813"));
    listener->Start();
    sleep(10);
    int count = 0;
    int8_t invoke_id = 0;
    while(true)
    {
      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
      if (tsm)
      {
        char* s;
        /* SMSC = 79139860004, MSISDN=79139870001 */
        char ms1[] = "79139870001";
        char ms2[] = "79139872021";
        //if (++count % 2) s = "79139870001"; else s = "79139872021";
        if (++count % 2) s = ms1; else s = ms2;
        string ms(s); // mobile station MSISDN
        string sca("79139860004"); // service center address
        uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
        cllen = packSCCPAddress(cl, 1 /* E.164 */, sca.c_str() /* SMSC E.164 */, 8 /* SMSC SSN */);
        cdlen = packSCCPAddress(cd, 1 /* E.164 */, ms.c_str() /* MS   E.164 */, 6 /* MS   SSN */);

        SendRoutingInfoForSMReq inv(ms, true, sca);
        tsm->TInvokeReq(invoke_id++, 45, inv);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
      }
      sleep(10);
    }
    listener->Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, "libSuaTest::: catched unexpected exception [%s]", ex.what());
  }
  return 0;
}
