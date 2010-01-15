static char const ident[] = "$Id$";
#include "mtsmsme/sccp/SccpProcessor.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/comp/SendRoutingInfo.hpp"
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "logger/Logger.h"
#include <string>
#include <vector>

using smsc::mtsmsme::processor::SccpProcessor;
using smsc::mtsmsme::processor::RequestSender;
using smsc::mtsmsme::processor::Request;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::mtsmsme::comp::SendRoutingInfoReq;
using smsc::mtsmsme::comp::SendRoutingInfoReqV2;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::locationInfoRetrievalContext_v2;
using smsc::mtsmsme::processor::locationInfoRetrievalContext_v3;
using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::TSMSTAT;
using smsc::mtsmsme::processor::BeginMsg;
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::dump;
using smsc::core::threads::Thread;
using smsc::sms::Address;
using smsc::logger::Logger;
using std::vector;
using std::string;

static char msca[] = "791398699815"; // MSC address
static char vlra[] = "791398699813"; //VLR address
static char hlra[] = "791398699814"; //HLR address

//static char abnt[] = "85297023844"; //abonent MSISDN
// 3G 898520300400362096GP601G - failure, IMSI contains only Hong Kong MCC and zeroes, returned MSRN is from German numbers

//static char abnt[] = "85294570103"; //abonent MSISDN
// SmartTone 060809142018208E4H4 - working

//static char abnt[] = "96280994"; //abonent MSISDN
// New World Mobility  89852101206020123344 - didn't registered in MTS 

//static char abnt[] = "85267074004"; //abonent MSISDN
// PEOPLES - no resp from peer

//static char abnt[] = "?????"; //abonent MSISDN
// PCCW - didn't registered in MTS

//static char abnt[] = "6583962854"; //abonent MSISDN
// Starhuv - working

//static char abnt[] = "375296849152"; //abonent MSISDN
// Chahovcski v pribaltike 

static char abnt[] = "639177949582"; //abonent MSISDN Omar Philippines
//static char abnt[] = "8613121567819"; //abonent MSISDN
//static char abnt[] = "8613810978987"; //abonent MSISDN
// Omar

//static char abnt[] = "79137247775"; //abonent MSISDN
// Kozlinsky in Europe

//static char abnt[] = "79096292608"; //abonent MSISDN
// Friend of AD in thai

static uint8_t userid = 44;
static uint8_t ssn = 192;

static Logger *logger = 0;
class EmptyRequestSender: public RequestSender {
  virtual bool send(Request* request)
  {
    request->setSendResult(0); return true;
  }
};
class EmptySubscriberRegistrator: public SubscriberRegistrator {
  public:
    EmptySubscriberRegistrator(TCO* _tco) : SubscriberRegistrator(_tco) {}
    virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period) {}
    virtual int  update(Address& imsi, Address& msisdn, Address& mgt) {return 1;}
    virtual bool lookup(Address& msisdn, Address& imsi, Address& msc) {return false;}
};
class GopotaListener: public SccpProcessor, public Thread {
  public:
    GopotaListener(TCO* _tco, SubscriberRegistrator* _reg) : SccpProcessor(_tco,_reg) {}
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

  try
  {
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("srireq");
    smsc_log_info(logger, "SRI Requester");

    EmptyRequestSender fakeSender;
    TCO mtsms(10000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms, &fakeHLR);
    listener.configure(userid, ssn, Address((uint8_t)strlen(msca), 1, 1, msca),
           Address((uint8_t)strlen(vlra), 1, 1, vlra),
           Address((uint8_t)strlen(hlra), 1, 1, hlra));
    listener.Start();
    sleep(10);
    // fill SCCP addresses for SRI sending, FROM = MSC GT, TO = HLR = MSISDN
    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
    cdlen = packSCCPAddress(cd, 1 /* E.164 */, abnt /* ABNT E.164 */, 6 /* HLR SSN */);
    cllen = packSCCPAddress(cl, 1 /* E.164 */, msca /* MSC  E.164 */, 8 /* MSC SSN */);
    {
      sleep(10);
      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(locationInfoRetrievalContext_v2);
      if (tsm)
      {
        string msisdn(abnt);
        string fromwho(msca);
        SendRoutingInfoReqV2 req(msisdn,fromwho);
        tsm->TInvokeReq( 1 /* invoke_id */, 22 /* SRI */, req);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
      }
    }
    while(true)
    {
      sleep(1);
    }
    listener.Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " cought unexpected exception [%s]", ex.what());
  }
  return 0;
}
