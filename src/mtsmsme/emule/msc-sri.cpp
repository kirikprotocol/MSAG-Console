static char const ident[] = "$Id$";
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
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::util::packSCCPAddress;
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

namespace smsc{namespace mtsmsme{namespace processor{
unsigned ConvertText27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,unsigned* elen,
  unsigned offset,unsigned buflen);
}}}
using smsc::mtsmsme::processor::ConvertText27bit;

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
    string msca("791398600045"); // MSC address
    string sca("79139860005"); // service center address
    string vlra("791398600043"); //VLR address
    string hlra("791398600044"); //HLR address
    EmptyRequestSender fakeSender;
    TCO mtsms(10000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms, &fakeHLR);
    listener.configure(44, 192, Address((uint8_t)msca.length(), 1, 1, msca.c_str()),
        Address((uint8_t)vlra.length(), 1, 1, vlra.c_str()),
        Address((uint8_t)hlra.length(), 1, 1, hlra.c_str()));
    listener.Start();
    sleep(10);
    // fill SCCP addresses for SMS sending, FROM = MSC GT, TO = SMSC GT
    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
    cdlen = packSCCPAddress(cd, 1 /* E.164 */, sca.c_str() /* SMSC E.164 */, 8 /* SMSC SSN */);
    cllen = packSCCPAddress(cl, 1 /* E.164 */, msca.c_str() /* MSC E.164 */, 8 /* MSC SSN */);
    {
      sleep(10);
      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(locationInfoRetrievalContext_v3);
      if (tsm)
      {
        SendRoutingInfoReq req("79131273996",msca);
        tsm->TInvokeReq( 1 /* invoke_id */, 22 /* SRI */, req);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
      }
      else
      {
      }
    }
    listener.Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " cought unexpected exception [%s]", ex.what());
  }
  return 0;
}
