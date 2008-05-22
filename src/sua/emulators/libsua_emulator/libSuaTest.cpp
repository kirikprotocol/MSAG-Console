#include <sua/libsua/SuaApiFactory.hpp>
#include <sua/libsua/MessageProperties.hpp>
#include <sua/libsua/MessageInfo.hpp>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

#define TRANS_TYPE 0
#define NUM_PLAN 0x10
#define ENC_SCHEME 0x01
#define NATURE_OF_ADDR 0x04

int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("init");

  const char* cfgFile;
  bool isReadOnlyClient = false;
  if ( argc < 2 )
    cfgFile = "libsua.xml";
  else {
    if (argc == 3) {
      cfgFile = argv[2];
      if ( !strcmp(argv[1], "-c"))
        isReadOnlyClient = true;
    } else
      cfgFile = argv[1];
  }

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
      suaApi.bind(i);
    }

    
    for(int i=0; i < suaApi.sua_getConnectsCount(); ++i) {
      if ( !isReadOnlyClient ) {
        libsua::MessageProperties msgProperties;
        msgProperties.returnOnError = true;
        msgProperties.hopCount = 2;
        msgProperties.fieldsMask = libsua::MessageProperties::SET_HOP_COUNT;

        uint8_t calledAddress[] = { 0x10, 0x00, TRANS_TYPE, NUM_PLAN | ENC_SCHEME, NATURE_OF_ADDR,
                                    0x97, 0x31, 0x89, 0x06, 0x00, 0x02 };

        uint8_t callingAddress[] = { 0x10, 0x00, TRANS_TYPE, NUM_PLAN | ENC_SCHEME, NATURE_OF_ADDR,
                                     0x97, 0x31, 0x89, 0x06, 0x00, 0x01 };

        // some application data
        const uint8_t messageData[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

        suaApi.unitdata_req(messageData, sizeof(messageData), calledAddress, sizeof(calledAddress), callingAddress, sizeof(callingAddress), msgProperties, 0);
      }

      smsc_log_info(logger, "waiting for a new message");
      libsua::MessageInfo msgInfo;
      suaApi.msgRecv(&msgInfo);

      smsc_log_info(logger, "got new message=[%s], messageType=[%d] from connection=[%d]", hexdmp(msgInfo.msgData.get(), msgInfo.msgData.GetPos()).c_str(), msgInfo.messageType, msgInfo.suaConnectNum);

      smsc_log_info(logger, "libSuaTest::: call sua_unbind(connectNum=%d)", i);
      suaApi.unbind(i);

      smsc_log_info(logger, "libSuaTest::: call sua_disconnect(connectNum=%d)", i);
      suaApi.sua_disconnect(i);
    }

  } catch (std::exception& ex) {
    smsc_log_error(logger, "libSuaTest::: catched unexpected exception [%s]", ex.what());
  }
  return 0;
}
