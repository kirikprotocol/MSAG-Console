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
using smsc::mtsmsme::processor::shortMsgMoRelayContext_v2;
using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::TSMSTAT;
using smsc::mtsmsme::comp::MoForwardSmReq;
using smsc::mtsmsme::processor::BeginMsg;
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::dump;

static char msca[] = "791398600223"; // MSC address
static char vlra[] = "79139860004"; //VLR address
static char hlra[] = "79139860004"; //HLR address
static char sca[]  = "79139869990"; // service center address
static char rndmsto_pattern[]   = "791398600222%04d";
static char rndmsfrom_pattern[] = "791398600221%04d";
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
        millisleep(delay-msgproc-overdelay);
        overdelay=(gethrtime()-msgstart)/1000000-(delay-msgproc-overdelay);
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
    smsc_log_info(logger, "MO SMS generator");
    smsc_log_error(logger,"sizeof(MoForwardSmReq)=%d",sizeof(MoForwardSmReq));
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

    listener.configure(43, 191, Address(strlen(msca), 1, 1, msca),
        Address(strlen(vlra), 1, 1, vlra),
        Address(strlen(hlra), 1, 1, hlra));
    listener.Start();
    sleep(10);
    uint32_t smscount = 0;
    // fill SCCP addresses for SMS sending, FROM = MSC GT, TO = SMSC GT
    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
    cdlen = packSCCPAddress(cd, 1 /* E.164 */, sca /* SMSC E.164 */, 8 /* SMSC SSN */);
    cllen = packSCCPAddress(cl, 1 /* E.164 */, msca /* MSC E.164 */, 8 /* MSC SSN */);
    while(true)
    {
      /*
       pos = snprintf(rndmsto,sizeof(rndmsto),"791398699814%04d",randint(100,9999));
       rndmsto[pos] = 0;
       pos = snprintf(rndmsfrom,sizeof(rndmsfrom),"791398699814%04d",randint(0,99));
       rndmsfrom[pos] = 0;
       string msto(rndmsto); // B-subsriber e.g. "7913986998140100"
       string msfrom(rndmsfrom); // A-subscriber e.g. "7913986998140001

       //prepare SMS
       //FIRST MODE Mega testovajf hren'
       //0000000    21  2e  0f  91  97  31  89  96  89  41  33  f3  00  08  ff  28
       //0000020    04  1c  04  35  04  33  04  30  00  20  04  42  04  35  04  41
       //0000040    04  42  04  3e  04  32  04  30  04  4f  00  20  04  45  04  40
       //0000060    04  35  04  3d  04  4c  00  21
       unsigned char smstext[] = { 0x04,0x1c,0x04,0x35,0x04,0x33,0x04,0x30,
       0x00,0x20,0x04,0x42,0x04,0x35,0x04,0x41,
       0x04,0x42,0x04,0x3e,0x04,0x32,0x04,0x30,
       0x04,0x4f,0x00,0x20,0x04,0x45,0x04,0x40,
       0x04,0x35,0x04,0x3d,0x04,0x4c,0x00,0x21 };
       OCTET_STRING_DECL(dest,20);
       ZERO_OCTET_STRING(dest);
       dest.size = packNumString2BCD91(dest.buf,msto.c_str(), msto.length());
       vector<unsigned char> ui;
       ui.push_back(0x11); //MI
       ui.push_back(0x2E); //TP Message Reference
       ui.push_back(msto.length()); // TP-Destination-Address length in digits
       ui.insert(ui.end(),dest.buf,dest.buf+dest.size); //TP-Destination-Address
       ui.push_back(0); //TP-Protocol-Identifier
       ui.push_back(0x08); //TP-Data_Coding-Scheme
       ui.push_back(0xFF); //TP-Validity-Period
       ui.push_back(sizeof(smstext)); //TP-User-Data-Length
       ui.insert(ui.end(), smstext, smstext + sizeof(smstext)); // TP-User-Data
       MoForwardSmReq mosms(sca,msfrom,ui);
       */

      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(shortMsgMoRelayContext_v2);
      //mtsms->
      if (tsm)
      {
        char rndmsto[20] = {0};
        char rndmsfrom[20] = {0};
        int pos;
        pos = snprintf(rndmsto,sizeof(rndmsto),rndmsto_pattern,randint(0,9999));
        rndmsto[pos] = 0;
        pos = snprintf(rndmsfrom,sizeof(rndmsfrom),rndmsfrom_pattern,randint(0,9999));
        rndmsfrom[pos] = 0;
        string msto(rndmsto); // B-subsriber
        string msfrom(rndmsfrom); // A-subscriber
        //prepare SMS count in message body
        char smstextlatin1[20] = {0}; int tpos;
        tpos = snprintf(smstextlatin1,sizeof(smstextlatin1),"%010d",++smscount);
        smstextlatin1[tpos] = 0;
        unsigned char smstext[20] = {0};
        unsigned int escaped_len = 0; // TP-User-Data-Length
        //unsigned maxlen=(unsigned)(ET96MAP_MAX_SIGNAL_INFO_LEN-(pdu_ptr+1-(pdu->signalInfo+1)));
        int _newbuflen = ConvertText27bit(
            (unsigned char *)smstextlatin1,strlen(smstextlatin1),
            smstext,&escaped_len,0,sizeof(smstext));

        OCTET_STRING_DECL(dest,20);
        ZERO_OCTET_STRING(dest);
        dest.size = packNumString2BCD91(dest.buf,msto.c_str(), msto.length());
        vector<unsigned char> ui;
        ui.push_back(0x11); //MI
        ui.push_back(0x2E); //TP Message Reference
        ui.push_back(msto.length()); // TP-Destination-Address length in digits
        ui.insert(ui.end(),dest.buf,dest.buf+dest.size); //TP-Destination-Address
        ui.push_back(0); //TP-Protocol-Identifier
        ui.push_back(0x00); //TP-Data_Coding-Scheme
        ui.push_back(0xFF); //TP-Validity-Period
        ui.push_back(escaped_len); //TP-User-Data-Length
        ui.insert(ui.end(), smstext, smstext + _newbuflen); // TP-User-Data
        MoForwardSmReq mosms(sca,msfrom,ui);

        tsm->TInvokeReq( 1 /* invoke_id */, 46 /* mo-ForwardSM */, mosms);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
        smsc_log_debug(logger,"sent %010d",smscount);
        invokation_count++;
      //struct timespec delay = { 0, 5000000}; // nonoseconds
      //nanosleep(&delay, 0);
      }
      else
      {
      //sleep(1);
      struct timespec delay = { 0, 500000000}; // nonoseconds
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
