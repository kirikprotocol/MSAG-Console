static char const ident[] = "$Id$";
#include <string>
#include <vector>
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "util/sleep.h"
#include "mtsmsme/sua/SuaProcessor.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/comp/MoForwardSm.hpp"
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"

using std::vector;
using std::string;
using smsc::core::threads::Thread;
using smsc::logger::Logger;
using smsc::sms::Address;
using smsc::util::millisleep;
using smsc::mtsmsme::processor::SuaProcessor;
using smsc::mtsmsme::processor::RequestSender;
using smsc::mtsmsme::processor::Request;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::TSMSTAT;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::BeginMsg;
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::dump;

static char msca[] = "791398600063"; // MSC address
static char vlra[] = "79139860004"; //VLR address
static char hlra[] = "79139860004"; //HLR address
static char sca[]  = "79139860005"; // service center address
static char rndmsto_pattern[]   = "791398600062%04d";
static char rndmsfrom_pattern[] = "791398600061%04d";
static int speed = 600;


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
class TrafficShaper: public SccpSender {
  private:
    SccpSender* adaptee;
    int delay;
    int overdelay;
    hrtime_t msgstart;
    void shape()
    {
      hrtime_t msgproc=gethrtime()-msgstart;
      msgproc/=1000000;
      if(delay>msgproc+overdelay)
      {
        msgstart=gethrtime();
        millisleep((unsigned)(delay-msgproc-overdelay));
        overdelay=(int)((gethrtime()-msgstart)/1000000-(delay-msgproc-overdelay));
      }else
      {
        overdelay-=delay;
        if(overdelay<0)overdelay=0;
      }
    }
  public:
    TrafficShaper(SccpSender* _adaptee, int _speed) :
      adaptee(_adaptee)
    {
      delay=1000/_speed; overdelay = 0;
    }
    void send(uint8_t cdlen, uint8_t *cd, uint8_t cllen, uint8_t *cl,
        uint16_t ulen, uint8_t *udp)
    {
      msgstart=gethrtime();
      adaptee->send(cdlen,cd,cllen,cl,ulen,udp);
      shape();
    }
};
int randint(int min, int max)
{
  return min+int((max-min+1)*rand()/(RAND_MAX+1.0));
}
static uint64_t invokation_count;
class StatFlusher: public Thread {
  private:
    uint64_t temp_counter;
    TSMSTAT stat;
  public:
    StatFlusher(): temp_counter(invokation_count)
    {
      TSM::getCounters(stat);
    }
    virtual int Execute()
    {
      struct timespec delay = {0, 995000000}; // 955ms
      while (true)
      {
        nanosleep(&delay,0);

        TSMSTAT statnow;
        TSM::getCounters(statnow);
        smsc_log_info(logger,"dlg cre/del/cnt = %d/%d/%d, sent %lld",
            statnow.objcreated - stat.objcreated,
            statnow.objdeleted - stat.objdeleted,
            statnow.objcount,
            invokation_count - temp_counter);

        stat = statnow;
        temp_counter = invokation_count;
      }
      return 0;
    }
};
int main(int argc, char** argv)
{
  try
  {
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("mosmgen");
    smsc_log_info(logger, "SRI4SM generator");
    StatFlusher trener;
    trener.Start();
    EmptyRequestSender fakeSender;
    TCO mtsms(10000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms, &fakeHLR);

    //inject traffic shaper
    TrafficShaper shaper((SccpSender*)&listener, speed);
    mtsms.setSccpSender((SccpSender*)&shaper);

    listener.configure(43, 191, Address((uint8_t)strlen(msca), 1, 1, msca),
        Address((uint8_t)strlen(vlra), 1, 1, vlra),
        Address((uint8_t)strlen(hlra), 1, 1, hlra));
    listener.Start();
    sleep(10);
    uint32_t smscount = 0;
    while(true)
    {
      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
      if (tsm)
      {
        char rndmsto[20] = { 0}; int pos;
        pos = snprintf(rndmsto,sizeof(rndmsto),rndmsto_pattern,randint(0,9999));
        rndmsto[pos] = 0;
        string msca_str(msca);
        string rndmsto_str(rndmsto);
        uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
        // fill SCCP addresses for SRI4SM , FROM = MSC GT, TO = MOBILE STATION GT
        cllen = packSCCPAddress(cl, 1 /* E.164 */, msca /* SMSC E.164 */, 8 /* SMSC SSN */);
        cdlen = packSCCPAddress(cd, 1 /* E.164 */, rndmsto /* MS   E.164 */, 6 /* MS   SSN */);

        SendRoutingInfoForSMReq sri4sm(rndmsto_str, true, msca_str);
        tsm->TInvokeReq( 1 /* invoke_id */, 45, sri4sm);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
        invokation_count++;
      //struct timespec delay = { 0, 495000000}; // nonoseconds
      //nanosleep(&delay, 0);
      }
      else
      {
      struct timespec delay = { 0, 500*1000*1000}; // nonoseconds
      nanosleep(&delay, 0);
      }
    }
    listener.Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " cought unexpected exception [%s]", ex.what());
  }
  return 0;
}
