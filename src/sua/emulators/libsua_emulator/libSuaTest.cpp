#include <sua/libsua/SuaApiFactory.hpp>
#include <sua/communication/sua_messages/ProtocolClass.hpp>
#include <sua/libsua/MessageProperties.hpp>
#include <sua/libsua/MessageInfo.hpp>
#include <sua/libsua/SCCPAddress.hpp>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);
//#include <sua/libsua/SuaApi.hpp>
//#include <sua/libsua/SuaUser.hpp>

int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("init");

  const char* cfgFile;
  if ( argc < 2 )
    cfgFile = "libsua.xml";
  else
    cfgFile = argv[1];

  try {
    smsc::util::config::Manager::init(cfgFile);
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    smsc::util::config::ConfigView libsuaConfigView(manager, "sua");

    libsua::SuaApiFactory::init();
    libsua::SuaApi& suaApi = libsua::SuaApiFactory::getSuaApiIface();
    suaApi.sua_init(&libsuaConfigView);

    for(int i=0; i < suaApi.sua_getConnectsCount(); ++i) {
      smsc_log_info(logger, "libSuaTest::: call sua_connect(connectNum=%d)", i);
      suaApi.sua_connect(i);
      smsc_log_info(logger, "libSuaTest::: call sua_bind(connectNum=%d)", i);
      suaApi.sua_bind(i);
    }

    for(int i=0; i < suaApi.sua_getConnectsCount(); ++i) {
      libsua::SCCPAddress srcAddress(sua_messages::GlobalTitle(sua_messages::GlobalTitle::TRANSLATION_TYPE_UNKNOWN, sua_messages::GlobalTitle::E164_NUMBERING_PLAN, sua_messages::GlobalTitle::INTERNATIONAL_NUMBER, "79139860001"));
      libsua::SCCPAddress dstAddress(sua_messages::GlobalTitle(sua_messages::GlobalTitle::TRANSLATION_TYPE_UNKNOWN, sua_messages::GlobalTitle::E164_NUMBERING_PLAN, sua_messages::GlobalTitle::INTERNATIONAL_NUMBER, "79139860002"));
      libsua::MessageProperties msgProperties;
      msgProperties.protocolClass = sua_messages::CLASS0_CONNECIONLESS;
      msgProperties.returnOnError = true;
      msgProperties.sequenceControlValue = 0;
      msgProperties.fieldsMask = libsua::MessageProperties::SET_PROT_CLASS | libsua::MessageProperties::SET_RETURN_ON_ERROR | libsua::MessageProperties::SET_SEQUENCE_CONTROL;

      // some application data
      const uint8_t messageData[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

      suaApi.sua_send_cldt(messageData, sizeof(messageData), srcAddress, dstAddress, msgProperties, 0);

      libsua::MessageInfo msgInfo;
      suaApi.sua_recvmsg(&msgInfo);

      smsc_log_info(logger, "got message=[%s]", hexdmp(msgInfo.msgData.get(), msgInfo.msgData.getSize()).c_str());

      smsc_log_info(logger, "libSuaTest::: call sua_unbind(connectNum=%d)", i);
      suaApi.sua_bind(i);

      smsc_log_info(logger, "libSuaTest::: call sua_disconnect(connectNum=%d)", i);
      suaApi.sua_disconnect(i);
    }

  } catch (std::exception& ex) {
    smsc_log_error(logger, "libSuaTest::: catched unexpected exception [%s]", ex.what());
  }
  return 0;
}
