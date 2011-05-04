static char const ident[] = "$Id$";
#include <string>
#include <vector>
#include "core/threads/Thread.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"
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
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
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

static char msca[20] = "791398699873"; // MSC address
static char vlra[20] = "79139860004"; //VLR address
static char hlra[20] = "79139860004"; //HLR address
static int  ssn = 191; //SSN
static int  userid = 43; //common part user id
//static char sca[]  = "791398699876"; // service center address SCS HD
static char sca[20]  = "79139869990"; // service center address
static char rndmsto_pattern[]   = "791398699872%04d";
static char rndmsfrom_pattern[] = "791398699871%04d";
static int speed = 1450;
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
class TrafficShaper: public SccpSender {
  private:
    SccpSender* adaptee;
    int delay;
    int overdelay;
    hrtime_t msgstart;
    bool slowstartmode;
    int slowstartperiod; //in seconds
    int speed;
    struct timeval slow_start;
    void adjustdelay()
    {
      if (slowstartmode)
      {
        timeval now;
        if (!slow_start.tv_sec)
          gettimeofday(&slow_start,NULL);
        gettimeofday(&now,NULL);
        if (slow_start.tv_sec + slowstartperiod < now.tv_sec)
        {
          slowstartmode = false;
          delay = 1000000/speed;
        }
        else
        {
          delay = 1000000/(1+(speed-1)*(now.tv_sec-slow_start.tv_sec)/slowstartperiod);
        }
      }
    }
    void shape()
    {
      adjustdelay();
      hrtime_t msgproc=gethrtime()-msgstart;
      msgproc/=1000;
      if(delay>msgproc+overdelay)
      {
        int toSleep=delay-msgproc-overdelay;
        msgstart=gethrtime();
        millisleep(toSleep/1000);
        overdelay=(gethrtime()-msgstart)/1000-toSleep;
      }else
      {
        overdelay-=delay-(int)msgproc;
      }
    }
  public:
    TrafficShaper(SccpSender* _adaptee, int _speed,int _slowstartperiod) :
      adaptee(_adaptee),slowstartmode(false),slowstartperiod(_slowstartperiod),
      overdelay(0)
    {
      if (slowstartperiod) slowstartmode = true;
      slow_start.tv_sec = 0;
      delay = 1000000/_speed;
      speed = _speed;
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
class ToolConfig {
  private:
    int cfgdelay;
    Logger* logger;
  public:
    ToolConfig(Logger* _logger):logger(_logger){}
    void read(Manager& manager)
    {
      char* tmp_str;
      if (!manager.findSection("msc-mosm"))
        throw ConfigException("\'msc-mosm\' section is missed");

      ConfigView view(manager, "msc-mosm");

      try { tmp_str = view.getString("sca");
            strcpy(sca,tmp_str);
      } catch (ConfigException& exc) {
        throw ConfigException("\'sca\' is unknown or missing");
      }

      try { speed = view.getInt("speed");
      } catch (ConfigException& exc) {
        throw ConfigException("\'speed\' is unknown or missing");
      }

      try { slowstartperiod = view.getInt("slowstartperiod");
      } catch (ConfigException& exc) {
        throw ConfigException("\'slowstartperiod\' is unknown or missing");
      }
    }
};
class SccpConfig {
  private:
    Logger* logger;
  public:
    SccpConfig(Logger* _logger):logger(_logger){}
    void read(Manager& manager)
    {
      char* tmp_str;
      Address tmp_addr;

      if (!manager.findSection("sccp"))
        throw ConfigException("\'sccp\' section is missed");

      ConfigView sccpConfig(manager, "sccp");

      try { userid = sccpConfig.getInt("user_id");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_id\' is unknown or missing");
      }

      try { ssn = sccpConfig.getInt("user_ssn");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_ssn\' is unknown or missing");
      }

      try {
        tmp_str = sccpConfig.getString("msc_gt");
        tmp_addr = Address(tmp_str);
        tmp_addr.getValue(msca);
      } catch (ConfigException& exc) {
        throw ConfigException("\'msc_gt\' is unknown or missing");
      }

      try {
        tmp_str = sccpConfig.getString("vlr_gt");
        tmp_addr = Address(tmp_str);
        tmp_addr.getValue(vlra);
      } catch (ConfigException& exc) {
        throw ConfigException("\'vlr_gt\' is unknown or missing");
      }

      try {
        tmp_str = sccpConfig.getString("hlr_gt");
        tmp_addr = Address(tmp_str);
        tmp_addr.getValue(hlra);
      } catch (ConfigException& exc) {
        throw ConfigException("\'hlr_gt\' is unknown or missing");
      }
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
    SccpConfig sccpcfg(logger);
    sccpcfg.read(manager);


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

    listener.configure(userid, ssn, Address((uint8_t)strlen(msca), 1, 1, msca),
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
        ui.push_back(0x03); //TP-Validity-Period = 20 minutes
        //ui.push_back(0xFF); //TP-Validity-Period = 63 weeks
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
