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
#include "mtsmsme/emule/TrafficShaper.hpp"

using std::vector;
using std::string;
using smsc::core::threads::Thread;
using smsc::logger::Logger;
using smsc::sms::Address;
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
using smsc::mtsmsme::processor::TrafficShaper;

static char msca[] = "791398699873"; // MSC address
static char vlra[] = "79139860004"; //VLR address
static char hlra[] = "79139860004"; //HLR address
//static char sca[]  = "791398699876"; // service center address SCS HD
static char sca[]  = "79139869990"; // service center address
static char rndmsto_pattern[]   = "791398699872%04d";
static char rndmsfrom_pattern[] = "791398699871%04d";
static int speed = 1;
static int slowstartperiod = 60; //in seconds


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
class Phone {
    static uint8_t PartsNumber;
  private:
    uint8_t MR; //TP Message Reference
    uint8_t MID;
    uint8_t MP;
    uint8_t MPN;
  public:
    uint8_t getMessageReference() { return MR++; }
    uint8_t get();
};
class Flooder {
    string pp;
  public:
    string& getPhone() {return pp;}
};
void makepdu(char* rndmsfrom,vector<unsigned char>& ui)
{
  Phone abnt;
    uint8_t MR = abnt.getMessageReference(); //TP Message Reference
    uint8_t MID;
    uint8_t MP;
    uint8_t MPN;

    int pos;
    pos = snprintf(rndmsfrom, 20, rndmsfrom_pattern, randint(0, 9999));
    rndmsfrom[pos] = 0;

    char rndmsto[20] = {0};
    pos = snprintf(rndmsto, sizeof (rndmsto), rndmsto_pattern, randint(0, 9999));
    rndmsto[pos] = 0;
    string msto(rndmsto); // B-subsriber

    OCTET_STRING_DECL(dest,20);
    ZERO_OCTET_STRING(dest);
    dest.size = (int)(packNumString2BCD91(dest.buf, msto.c_str(), (unsigned )(msto.length())));
    ui.push_back(0x41); //MI = No reply path, UDH is present, SRR not requested, validity period not present, accept duplicates, SUBMIT
    ui.push_back(MR); //TP Message Reference
    ui.push_back((uint8_t)(msto.length())); // TP-Destination-Address length in digits
    ui.insert(ui.end(), dest.buf, dest.buf + dest.size); //TP-Destination-Address
    ui.push_back(0); //TP-Protocol-Identifier
    ui.push_back(0x08); //TP-Data_Coding-Scheme = UCS2
    //ui.push_back(0xFF); //TP-Validity-Period
    ui.push_back(0x8c); //TP-User-Data-Length = 140 octets
    ui.push_back(0x05); // UDH length
    ui.push_back(0x00); // Concat IE tag
    ui.push_back(0x03); // Concat IE len
    ui.push_back(MID); // Concat Message identifier
    ui.push_back(MP); // Concat mesaage parts total
    ui.push_back(MPN); // Concat mesaage part number
    // TP-User-Data SMS TEXT
    for (int i = 0; i < 67; ++i) // 140 - 6 (UDH len + UDH) / 2 (UCS2)
    {
      ui.push_back(0x00);
      ui.push_back(MPN);
    }
}

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
    TCO mtsms(100000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms, &fakeHLR);

    //inject traffic shaper
    TrafficShaper shaper((SccpSender*)&listener, speed,slowstartperiod);
    mtsms.setSccpSender((SccpSender*)&shaper);

    listener.configure(43, 191, Address((uint8_t)strlen(msca), 1, 1, msca),
        Address((uint8_t)strlen(vlra), 1, 1, vlra),
        Address((uint8_t)strlen(hlra), 1, 1, hlra));
    listener.Start();
    sleep(10);
    uint32_t smscount = 0;
    // fill SCCP addresses for SMS sending, FROM = MSC GT, TO = SMSC GT
    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
    cdlen = packSCCPAddress(cd, 1 /* E.164 */, sca /* SMSC E.164 */, 8 /* SMSC SSN */);
    cllen = packSCCPAddress(cl, 1 /* E.164 */, msca /* MSC E.164 */, 8 /* MSC SSN */);
    while(true)
    {

      TSM* tsm = 0;
      tsm = mtsms.TC_BEGIN(shortMsgMoRelayContext_v2);
      if (tsm)
      {
        ////
        char rndmsfrom[20] = {0};
        int pos;
        pos = snprintf(rndmsfrom,sizeof(rndmsfrom),rndmsfrom_pattern,randint(0,9999));
        rndmsfrom[pos] = 0;
        string msfrom(rndmsfrom); // A-subscriber
        //////
        vector<unsigned char> ui;
        makepdu(rndmsfrom,ui);
        MoForwardSmReq mosms(sca, msfrom, ui);
        tsm->TInvokeReq(1, 46, mosms);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
        smsc_log_debug(logger,"sent %010d",smscount);
        invokation_count++;
      }
      else
      {
      struct timespec delay = { 0, 500000000}; // Nanoseconds
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
