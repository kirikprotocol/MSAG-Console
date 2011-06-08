static char const ident[] = "$Id$";
#include <string>
#include <vector>
#include <list>
#include "core/threads/Thread.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"
#include "util/sleep.h"
#include "mtsmsme/sua/SuaProcessor.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/processor/MOFTSM.hpp"
#include "mtsmsme/comp/MoForwardSm.hpp"
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/TrafficShaper.hpp"

using std::vector;
using std::string;
using std::list;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
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
using smsc::mtsmsme::processor::MOFTSM;
using smsc::mtsmsme::processor::MOFTSMListener;
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
const int maxPhones=10000;
static int partsNumber = 3; //Maximum number of short messages in the concatenated short message
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
    bool going;
    uint64_t temp_counter;
    TSMSTAT stat;
  public:
    StatFlusher(): temp_counter(invokation_count),going(true)
    {
      TSM::getCounters(stat);
    }
    void Stop() {going = false;}
    virtual int Execute()
    {
      struct timespec delay = {0, 995000000}; // 955ms
      while (going)
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
class InvokationCount: public SccpSender {
  public:
    void send(uint8_t cdlen, uint8_t *cd, uint8_t cllen, uint8_t *cl, uint16_t ulen, uint8_t *udp)
    {
      invokation_count++;
    }
};
class Phone {
    //Maximum number of short messages in the concatenated short message
    static uint8_t MP;
  private:
    uint16_t phoneIndex; //randint(0, 9999)
    uint16_t destIndex;
    uint8_t MR; //TP Message Reference
    uint8_t MID; //Concatenated short message reference number
    uint8_t MPN; //Sequence number of the current short message
    enum State_e { phoneIDLE = 0, phoneBegin, phoneSent, phoneEnd, phoneWait };
    State_e state;
    static std::string getStateDescription(State_e state)
    {
      switch(state) {
        case phoneIDLE: return "phoneIDLE";
        case phoneBegin: return "phoneBegin";
        case phoneSent: return "phoneSent";
        case phoneEnd: return "phoneEnd";
        case phoneWait: return "phoneWait";
        default:return "UNKNOWN";
      }
    }
    void changeState(State_e _state)
    {
      State_e old_state = state;
      state = _state;
      switch(state) {
        case phoneIDLE: break;
        case phoneBegin:
          if (old_state == phoneIDLE) {
            MPN=1; MID++; destIndex = randint(0, maxPhones-1);
          }
          break;
        case phoneSent:
          break;
        case phoneEnd:
          MPN++;
          if (MPN <= MP)
            state = phoneWait;
          else
            state = phoneIDLE;
          break;
        case phoneWait:
          break;
      }
      smsc_log_debug(logger,"phone[%04d].state=%s MPN=%d MID=%d MR=%d",
          phoneIndex,getStateDescription(state).c_str(),MPN,MID,MR);
    }
  public:
    Phone():phoneIndex(0),MR(0),MID(0),MPN(0),state(phoneIDLE){}
    bool isWait() { return state == phoneWait; }
    void setIndex(uint16_t idx) { phoneIndex=idx;}
    uint16_t getIndex() { return phoneIndex; }
    uint8_t getMessageReference() { return MR++; }
    void begin(MOFTSM* tsm, uint8_t cdlen, uint8_t* cd, /* called party address */
                            uint8_t cllen, uint8_t* cl /* calling party address */)
    {
      changeState(phoneBegin);
      tsm->TBeginReq(cdlen, cd, cllen, cl);
    }
    void cont(MOFTSM* tsm)
    {
      changeState(phoneSent);
      char rndmsfrom[20] = { 0 };
      int pos;
      pos = snprintf(rndmsfrom, sizeof(rndmsfrom), rndmsfrom_pattern, phoneIndex);
      rndmsfrom[pos] = 0;
      string msfrom(rndmsfrom); // A-subscriber
      vector<unsigned char> ui;
      makepdu(ui);
      MoForwardSmReq mosms(sca, msfrom, ui);
      tsm->TInvokeReq(1, 46, mosms);
      tsm->TContReq();
    }
    void end(MOFTSM* tsm)
    {
      changeState(phoneEnd);
    }
    void makepdu(vector<unsigned char>& ui)
    {
      uint8_t MsgRef = getMessageReference(); //TP Message Reference

      char rndmsfrom[20] = { 0 };
      int pos;
      pos = snprintf(rndmsfrom, 20, rndmsfrom_pattern, phoneIndex);
      rndmsfrom[pos] = 0;

      char rndmsto[20] = { 0 };
      pos = snprintf(rndmsto, sizeof(rndmsto), rndmsto_pattern,destIndex);
      rndmsto[pos] = 0;
      string msto(rndmsto); // B-subsriber

      OCTET_STRING_DECL(dest,20);
      ZERO_OCTET_STRING(dest);
      dest.size = (int) (packNumString2BCD91(dest.buf, msto.c_str(),
          (unsigned) (msto.length())));
      //MI = No reply path, UDH is present, SRR not requested,
      // validity period field present - relative format
      // accept duplicates, SUBMIT
      ui.push_back(0x51);
      ui.push_back(MsgRef); //TP Message Reference
      ui.push_back((uint8_t) (msto.length())); // TP-Destination-Address length in digits
      ui.insert(ui.end(), dest.buf, dest.buf + dest.size); //TP-Destination-Address
      ui.push_back(0); //TP-Protocol-Identifier
      ui.push_back(0x08); //TP-Data_Coding-Scheme = UCS2
      ui.push_back(0x03); //TP-Validity-Period = 20 minutes
      int symbolCount = (MPN == MP) ? 33 : 67;  //bug in SMSC 3part -> 4 part
      ui.push_back(symbolCount*2+6); //TP-User-Data-Length = 140 octets
      //ui.push_back(0x8c); //TP-User-Data-Length = 140 octets
      ui.push_back(0x05); // UDH length
      ui.push_back(0x00); // Concat IE tag
      ui.push_back(0x03); // Concat IE len
      ui.push_back(MID); // Concat Message identifier
      ui.push_back(MP); // Concat message parts total
      ui.push_back(MPN); // Concat message part number
      // TP-User-Data SMS TEXT
      for (int i = 0; i < symbolCount; ++i) // 140 - 6 (UDH len + UDH) / 2 (UCS2)
      {
        ui.push_back(0x00);
        ui.push_back(MPN + '0');
      }
    }
};
uint8_t Phone::MP=partsNumber; //Maximum number of short messages in the concatenated short message

class Flooder : public MOFTSMListener, public Thread{
  private:
    bool going;
    Mutex lock;
    SccpSender& shaper;
    TCO& mtsms;
    uint32_t smscount;
    Phone* phones;
    list<uint16_t> idxqueue;
    // fill SCCP addresses for SMS sending, FROM = MSC GT, TO = SMSC GT
    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
  public:
    Flooder(SccpSender& _shaper, TCO& _mtsms): shaper(_shaper),mtsms(_mtsms)
    {
      going = true;
      smscount = 0;
      cdlen = packSCCPAddress(cd, 1 /* E.164 */, sca /* SMSC E.164 */, 8 /* SMSC SSN */);
      cllen = packSCCPAddress(cl, 1 /* E.164 */, msca /* MSC E.164 */, 8 /* MSC SSN */);
      phones = new Phone[maxPhones];
      for (int idx = 0; idx < maxPhones; ++idx)
      {
        phones[idx].setIndex((uint16_t) idx);
      }
    }
    ~Flooder() {delete []phones;}
    void sendpart(MOFTSM* tsm)
    {
      uint16_t idx;
      if (! idxqueue.empty())
      {
        MutexGuard g(lock);
        idx = idxqueue.front();
        idxqueue.pop_front();
      }
      else
      {
        idx = randint(0, maxPhones - 1);
      }
      tsm->addMOFTSMListener(this, idx);
      phones[idx].begin(tsm,cdlen, cd, cllen, cl);
      // call shaper
      shaper.send(0,0,0,0,0,0);
      smsc_log_debug(logger,"sent %010d",smscount);
    }
    // listener method
    // if sent part is not last in sequence
    // place phone in wait list
    virtual void end(MOFTSM* tsm,uint16_t idx)
    {
      phones[idx].end(tsm);
      if ( phones[idx].isWait())
      {
        MutexGuard g(lock);
        idxqueue.push_back(idx);
      }
    }
    void cont(MOFTSM* tsm,uint16_t phoneIndex)
    {
      phones[phoneIndex].cont(tsm);
    }
    void Stop() { going = false;}
    virtual int Execute()
    {
      while (going)
      {
        MOFTSM* tsm = 0;
        tsm = (MOFTSM*) mtsms.TC_BEGIN(shortMsgMoRelayContext_v2);
        if (tsm)
        {
          sendpart(tsm); // get random phone
        }
        else
        {
          struct timespec delay = { 0, 500000000 }; // Nanoseconds
          nanosleep(&delay, 0);
        }
      }
      return 0;
    }
};
class ToolConfig {
  private:
    int cfgspeed;
    int slow;
    Logger* logger;
  public:
    ToolConfig(Logger* _logger):logger(_logger){}
    void read(Manager& manager)
    {
      if (!manager.findSection("msc-mosm-mpart"))
        throw ConfigException("\'msc-mosm-mpart\' section is missed");

      ConfigView sccpConfig(manager, "msc-mosm-mpart");

      try { tmp_str = view.getString("sca");
            strcpy(sca,tmp_str);
      } catch (ConfigException& exc) {
        throw ConfigException("\'sca\' is unknown or missing");
      }

      try { cfgspeed = sccpConfig.getInt("speed");
      } catch (ConfigException& exc) {
        throw ConfigException("\'speed\' is unknown or missing");
      }
      speed = cfgspeed;

      try { slow = sccpConfig.getInt("slowstartperiod");
      } catch (ConfigException& exc) {
        throw ConfigException("\'slowstartperiod\' is unknown or missing");
      }
      slowstartperiod = slow;
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
    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();
    ToolConfig config(logger);
    config.read(manager);

    srand(time(NULL)); // set seed
    StatFlusher trener;
    EmptyRequestSender fakeSender;
    TCO mtsms(100000);
    EmptySubscriberRegistrator fakeHLR(&mtsms);
    mtsms.setRequestSender(&fakeSender);
    GopotaListener listener(&mtsms, &fakeHLR);


    mtsms.setSccpSender((SccpSender*)&listener);

    listener.configure(43, 191, Address((uint8_t)strlen(msca), 1, 1, msca),
        Address((uint8_t)strlen(vlra), 1, 1, vlra),
        Address((uint8_t)strlen(hlra), 1, 1, hlra));

    //inject traffic shaper
    InvokationCount shapecounter;
    TrafficShaper shaper(&shapecounter, speed,slowstartperiod);
    Flooder flood(shaper,mtsms);


    listener.Start();
    trener.Start();
    sleep(10);
    flood.Start();
    flood.WaitFor();
    listener.Stop();
    trener.Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " cought unexpected exception [%s]", ex.what());
  }
  return 0;
}

