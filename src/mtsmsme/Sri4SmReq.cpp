#include "eyeline/ss7na/libsccp/SccpApiFactory.hpp"
#include "eyeline/ss7na/libsccp/MessageProperties.hpp"
#include "eyeline/ss7na/libsccp/MessageInfo.hpp"
#include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/N_NOTICE_IND_Message.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include <string>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

#define TRANS_TYPE 0
#define NUM_PLAN 0x10
#define ENC_SCHEME 0x01
#define NATURE_OF_ADDR 0x04

using smsc::core::threads::Thread;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using std::string;
using namespace eyeline::ss7na;


static Logger *logger = 0;
class SuaListener : public Thread {
  private:
    libsua::SuaApi& api;
    TCO& tco;
    bool going;
  public:
    SuaListener(libsua::SuaApi& suaApi, TCO& _tco) :
      Thread(), api(suaApi), going(true), tco(_tco) {}
    virtual int Execute()
    {
      int result;
      libsua::MessageInfo message;
      while (going)
      {
        result = api.msgRecv(&message,1000);
        if (result == libsua::SuaApi::SOCKET_TIMEOUT)
          continue;
        if (result != libsua::SuaApi::OK)
        {
          smsc_log_error(logger,"MsgRecv failed: %d", result);
          going = 0;
          break;
        }
        smsc_log_debug(logger,
                         "got new message type=%d data[%d]={%s} from connection=%d",
                         message.messageType,message.msgData.getPos(),
                         dump(message.msgData.getPos(),message.msgData.get()).c_str(),
                         message.suaConnectNum);
        {
          switch ((int)message.messageType)
          {
            case libsua::SUAMessageId::N_UNITDATA_IND_MSGCODE :
            {
              //decode with libsua
              libsua::N_UNITDATA_IND_Message ind;
              ind.deserialize(message.msgData.get(), message.msgData.getPos());
              tco.NUNITDATA(ind.getCalledAddress().dataLen,
                            (uint8_t*)ind.getCalledAddress().data,
                            ind.getCallingAddress().dataLen,
                            (uint8_t*)ind.getCallingAddress().data,
                            ind.getUserData().dataLen,
                            (uint8_t*)ind.getUserData().data);
              break;
              //decode byself
              uint8_t* ptr = message.msgData.get();
              uint8_t fieldMask = *ptr++;
              if (fieldMask & 0x01) ptr += sizeof(uint32_t); //skip sequenct control
              uint8_t cdlen = *ptr++; uint8_t* cd = ptr; ptr +=cdlen;
              uint8_t cllen = *ptr++; uint8_t* cl = ptr; ptr +=cllen;
              uint16_t _ulen;
              memcpy(reinterpret_cast<uint8_t*>(&_ulen), ptr, sizeof(_ulen));
              uint16_t ulen = ntohs(_ulen); ptr += sizeof(_ulen);
              uint8_t* udp = ptr;
              //UNITDATA_IND
              tco.NUNITDATA(cdlen,cd,cllen,cl,ulen,udp);
              break;
            }
          }
        }
        message.msgData.setPos(0);
      }
      smsc_log_info(logger, "listener finished");
      return 0;
    }
    void Stop() { going = false; }
};
class SuaSender : public SccpSender {
  using namespace eyeline::ss7na;
  private:
    libsccp::SccpApi& api;
  public:
    SuaSender(libsua::SuaApi& suaApi) : api(suaApi) {}
    SuaSender(libs) {}
    void send(uint8_t cdlen, uint8_t *cd,
              uint8_t cllen, uint8_t *cl,
              uint16_t ulen, uint8_t *udp)
    {
      libsccp::MessageProperties msgProperties;
      msgProperties.setReturnOnError(true);
      msgProperties.setHopCount(2);
      libsccp::SccpApi& suaApi = libsccp::SccpApiFactory::getSccpApiIface();
      libsccp::SccpApi::ErrorCode_e res =
          suaApi.unitdata_req(udp, ulen,
                                    cd, cdlen,
                                    cl, cllen,
                                    msgProperties, 0);
      smsc_log_info(logger, "unitdata_req  with code %d",res);
    }
};
int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("sri4smreq");

  try
  {
    smsc_log_info(logger, "Send Routing Info For SM generator");
    smsc::util::config::Manager::init("sua.xml");
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    smsc::util::config::ConfigView libsuaConfigView(manager, "sua");

    libsccp::SccpApiFactory::init();
    libsccp::SccpApi& sccpApi = libsccp::SccpApiFactory::getSccpApiIface();
    sccpApi.sua_init(&libsuaConfigView);

    uint8_t ssnList[] = { 191 };

    for(int i=0; i < sccpApi.getConnectsCount(); ++i)
    {
      sccpApi.connect(i);
      smsc_log_info(logger, "connect(connectNum=%d)", i);
      int res;
      if ( ( res = sccpApi.bind(i,ssnList,static_cast<uint8_t>(sizeof(ssnList)))) != 0 )
        smsc_log_info(logger, "call bind(connectNum=%d) with code %d", i, res);
    }
    TCO* mtsms = new TCO(10);
    SuaListener* listener = new SuaListener(sccpApi,*mtsms);
    listener->Start();
    SccpSender* sccpsender = new SuaSender(sccpApi);
    mtsms->setSccpSender(sccpsender);
    int count = 0;
    int8_t invoke_id = 0;
    while(true)
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
      TSM* tsm = 0;
      tsm = mtsms->TC_BEGIN(shortMsgGatewayContext_v2);
      if (tsm)
      {
        SendRoutingInfoForSMReq* inv = new SendRoutingInfoForSMReq(ms, true, sca);
        tsm->TInvokeReq(invoke_id++, 45, *inv);
        tsm->TBeginReq(cdlen, cd, cllen, cl);
      }
      sleep(10);
    }
    for(int i=0; i < sccpApi.getConnectsCount(); ++i)
    {
      smsc_log_info(logger, "libSuaTest::: call unbind(connectNum=%d)", i);
      sccpApi.unbind(i);

      smsc_log_info(logger, "libSuaTest::: call disconnect(connectNum=%d)", i);
      sccpApi.disconnect(i);
    }
listener->Stop();
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, "libSuaTest::: catched unexpected exception [%s]", ex.what());
  }
  return 0;
}
