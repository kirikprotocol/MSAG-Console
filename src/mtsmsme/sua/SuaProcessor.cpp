static char const ident[] = "$Id$";
#include "mtsmsme/processor/Processor.h"
#include "mtsmsme/processor/util.hpp"
#include "logger/Logger.h"
#include "mtsmsme/sua/SuaProcessor.hpp"
#include "mtsmsme/processor/util.hpp"
#include "sms/sms.h"
#include <sua/libsua/SuaApiFactory.hpp>
#include <sua/libsua/MessageProperties.hpp>
#include <sua/libsua/MessageInfo.hpp>
#include <sua/libsua/messages/N_UNITDATA_IND_Message.hpp>
#include <sua/libsua/types.hpp>

using namespace std;
namespace smsc{
namespace mtsmsme{
namespace processor{

using smsc::sms::Address;
using smsc::sms::AddressValue;


using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::mtsmsme::processor::util::getReturnCodeDescription;
using smsc::mtsmsme::processor::util::dump;

#define MAXENTRIES 1000
#define MAXSEGM 272
#define TCINST 0
#define MGMT_VER 6
#define MCIERROR 999
int going;

Logger* logger = 0;
static Mutex lock;

static UCHAR_T USER = USER04_ID;
static UCHAR_T SSN = 191;

void SuaProcessor::configure(int user_id, int ssn, Address& msc, Address& vlr, Address& hlr)
{
  USER = user_id; SSN = ssn;
  coordinator->setAdresses(msc,vlr,hlr);
  registrator->configure(msc,vlr);
}

SuaProcessor::SuaProcessor()
{
  logger = Logger::getInstance("mt.sme.sua");
  //smsc_log_debug(logger,"\n**********************\n* SIBINCO MT SMS SME *\n**********************");
  sender = 0;
  coordinator = new TCO(100);
  coordinator->setSccpSender(this);
  registrator = new SubscriberRegistrator(coordinator);
  coordinator->setHLROAM(registrator);
}
HLROAM* SuaProcessor::getHLROAM() { return registrator; }
SuaProcessor::~SuaProcessor()
{
  delete(registrator);
  delete(coordinator);
}
TCO* SuaProcessor::getCoordinator() {return coordinator;}

void SuaProcessor::setRequestSender(RequestSender* _sender)
{
  MutexGuard g(lock);
  coordinator->setRequestSender(_sender);
}


void SuaProcessor::Stop()
{
  going = 0;
}
/*
 * @-> INIT -> SCCPBINDING -> WORKING
 */
enum State{
      INIT,
      SUABINDING,
      SUABINDERROR,
      WORKING
} state;

static std::string getStateDescription(State state)
{
  switch(state)
  {
    case INIT: return "INIT";
    case SUABINDING: return "SUABINDING";
    case SUABINDERROR: return "SUABINDERROR";
    case WORKING: return "WORKING";
  }
}
static void changeState(State nstate)
{
  state = nstate;
  smsc_log_debug(logger,
                 "RequestProcessor:%s",
                 getStateDescription(state).c_str());
}
extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);
static void suaHandleInd(libsua::MessageInfo& message, SccpUser& listener)
{
  smsc_log_debug(logger,
                 "got new message type=%d data[%d]={%s} from connection=%d",
                 message.messageType,message.msgData.GetPos(),
                 dump(message.msgData.GetPos(),message.msgData.get()).c_str(),
                 message.suaConnectNum);
  switch ((int)message.messageType)
  {
    case libsua::N_UNITDATA_IND_MSGCODE :
    {
      //decode with libsua
      libsua::N_UNITDATA_IND_Message ind;
      ind.deserialize(message.msgData.get(), message.msgData.GetPos());
      listener.NUNITDATA(ind.getCalledAddress().dataLen,
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
      listener.NUNITDATA(cdlen,cd,cllen,cl,ulen,udp);
      break;
    }
  }
}
int SuaProcessor::Run()
{
  int result;
  smsc::util::config::Manager::deinit();
  smsc::util::config::Manager::init("sua.xml");
  smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

  smsc::util::config::ConfigView libsuaConfigView(manager, "sua");

  libsua::SuaApiFactory::init();
  libsua::SuaApi& suaApi = libsua::SuaApiFactory::getSuaApiIface();
  suaApi.sua_init(&libsuaConfigView);

  changeState(INIT);
  for(int i=0; i < suaApi.sua_getConnectsCount(); ++i)
  {
    smsc_log_info(logger, "libSuaTest::: call sua_connect(connectNum=%d)", i);
    suaApi.sua_connect(i);
    smsc_log_info(logger, "libSuaTest::: call sua_bind(connectNum=%d)", i);
    suaApi.bind(i);
  }

  changeState(WORKING);
  going = 1;
  libsua::MessageInfo message;
  while (going)
  {
    switch (state)
    {
      case WORKING:
        registrator->process();
        break;
    }
    result = suaApi.msgRecv(&message,1000);
    if (result == libsua::SOCKET_TIMEOUT)
      continue;
    if (result != libsua::OK)
    {
      smsc_log_error(logger,"MsgRecv failed: %d", result);
      going = 0;
      break;
    }
    suaHandleInd(message,*coordinator);
    message.msgData.SetPos(0);//release buffer
//    smsc_log_info(logger, "got new message=[%s]", hexdmp(msgInfo.msgData.get(), msgInfo.msgData.GetPos()).c_str());
  }
  for(int i=0; i < suaApi.sua_getConnectsCount(); ++i)
  {
    smsc_log_info(logger, "libSuaTest::: call sua_unbind(connectNum=%d)", i);
    suaApi.unbind(i);
    smsc_log_info(logger, "libSuaTest::: call sua_disconnect(connectNum=%d)", i);
    suaApi.sua_disconnect(i);
  }
  return 0; //return result;
}

void SuaProcessor::send(uint8_t cdlen, uint8_t *cd,
                        uint8_t cllen, uint8_t *cl,
                        uint16_t ulen, uint8_t *udp)
{
  libsua::MessageProperties msgProperties;
  msgProperties.returnOnError = true;
  msgProperties.hopCount = 2;
  msgProperties.fieldsMask = libsua::MessageProperties::SET_HOP_COUNT;
  libsua::SuaApi& suaApi = libsua::SuaApiFactory::getSuaApiIface();
  suaApi.unitdata_req(udp, ulen,
                      cd, cdlen,
                      cl, cllen,
                      msgProperties, 0);
}
}//namespace processor
}//namespace mtsmsme
}//namespace smsc

