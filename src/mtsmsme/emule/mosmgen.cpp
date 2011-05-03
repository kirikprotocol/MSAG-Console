static char const ident[] = "$Id$";
#include "util/Exception.hpp"
#include "mtsmsme/processor/Processor.h"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/comp/MoForwardSm.hpp"
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "logger/Logger.h"
#include <string>
#include <vector>

using smsc::util::Exception;
using smsc::mtsmsme::processor::RequestProcessor;
using smsc::mtsmsme::processor::RequestProcessorFactory;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::mtsmsme::processor::RequestSender;
using smsc::mtsmsme::processor::Request;
using smsc::mtsmsme::processor::SubscriberRegistrator;
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
        smsc_log_info(logger,"stat flusher: sent %lld",invokation_count - temp_counter);
        temp_counter = invokation_count;

        //struct timespec delay = { 0, 995000000 }; // 955ms
        TSMSTAT statnow;
        TSM::getCounters(statnow);
        smsc_log_info(logger,"dlg cre/del/cnt = %d/%d/%d",
            statnow.objcreated - stat.objcreated,
            statnow.objdeleted - stat.objdeleted,
            statnow.objcount);
        stat = statnow;
        //nanosleep(&delay,0);
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
    uint32_t smscount = 0;
    // fill SCCP addresses for SMS sending, FROM = MSC GT, TO = SMSC GT
    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
    cdlen = packSCCPAddress(cd, 1 /* E.164 */, sca.c_str() /* SMSC E.164 */, 8 /* SMSC SSN */);
    cllen = packSCCPAddress(cl, 1 /* E.164 */, msca.c_str() /* MSC E.164 */, 8 /* MSC SSN */);
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
        pos = snprintf(rndmsto,sizeof(rndmsto),"791398600044%04d",randint(100,9999));
        rndmsto[pos] = 0;
        pos = snprintf(rndmsfrom,sizeof(rndmsfrom),"791398600044%04d",randint(0,99));
        rndmsfrom[pos] = 0;
        string msto(rndmsto); // B-subsriber e.g. "7913986998140100"
        string msfrom(rndmsfrom); // A-subscriber e.g. "7913986998140001
        //prepare SMS count in message body
        char smstextlatin1[20] = {0}; int tpos;
        tpos = snprintf(smstextlatin1,sizeof(smstextlatin1),"%010d",++smscount);
        smstextlatin1[tpos] = 0;
        unsigned char smstext[20] = {0};
        unsigned int escaped_len = 0; // TP-User-Data-Length
        //unsigned maxlen=(unsigned)(ET96MAP_MAX_SIGNAL_INFO_LEN-(pdu_ptr+1-(pdu->signalInfo+1)));
        int _newbuflen = ConvertText27bit(
            (unsigned char *)smstextlatin1,(unsigned)strlen(smstextlatin1),
            smstext,&escaped_len,0,(unsigned)sizeof(smstext));

        OCTET_STRING_DECL(dest,20);
        ZERO_OCTET_STRING(dest);
        dest.size = (int)packNumString2BCD91(dest.buf,msto.c_str(), (unsigned)msto.length());
        vector<unsigned char> ui;
        ui.push_back(0x11); //MI
        ui.push_back(0x2E); //TP Message Reference
        ui.push_back((uint8_t)msto.length()); // TP-Destination-Address length in digits
        ui.insert(ui.end(),dest.buf,dest.buf+dest.size); //TP-Destination-Address
        ui.push_back(0); //TP-Protocol-Identifier
        ui.push_back(0x00); //TP-Data_Coding-Scheme
        ui.push_back(0xFF); //TP-Validity-Period
        ui.push_back(escaped_len); //TP-User-Data-Length
        ui.insert(ui.end(), smstext, smstext + _newbuflen); // TP-User-Data
        MoForwardSmReq mosms(sca,msfrom,ui);

        tsm->TInvokeReq( 1 /* invoke_id */, 46 /* mo-ForwardSM */, mosms);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
        smsc_log_info(logger,"sent %010d",smscount);
        invokation_count++;
      //struct timespec delay = { 0, 500000000}; // nonoseconds
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
