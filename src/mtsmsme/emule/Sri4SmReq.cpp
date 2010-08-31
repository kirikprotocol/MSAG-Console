#include <string>
#include "core/threads/Thread.hpp"
#include "eyeline/ss7na/libsccp/SccpApiFactory.hpp"
#include "eyeline/ss7na/libsccp/MessageInfo.hpp"
#include "eyeline/ss7na/libsccp/MessageProperties.hpp"
#include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/N_NOTICE_IND_Message.hpp"
#include "util/config/XCFManager.hpp"
#include "eyeline/ss7na/libsccp/xcfg/LibSccpCfgReader.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/processor/util.hpp"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

#define TRANS_TYPE 0
#define NUM_PLAN 0x10
#define ENC_SCHEME 0x01
#define NATURE_OF_ADDR 0x04

using smsc::core::threads::Thread;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::sms::Address;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using std::string;
using namespace eyeline::ss7na;


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
int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("sri4smreq");
  try
  {
    TCO* mtsms = new TCO(10);
    SuaListener* listener = new SuaListener(sccpApi,*mtsms);

    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms, &fakeHLR);

    listener.configure(43, 191, Address((uint8_t)strlen(msca), 1, 1, msca),
            Address((uint8_t)strlen(vlra), 1, 1, vlra),
            Address((uint8_t)strlen(hlra), 1, 1, hlra));
    listener->Start();
    sleep(10);
    int count = 0;
    int8_t invoke_id = 0;
    while(true)
    {
      TSM* tsm = 0;
      tsm = mtsms->TC_BEGIN(shortMsgGatewayContext_v2);
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
