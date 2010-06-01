#include <stdio.h>
#include <stdlib.h>

#include "logger/Logger.h"
#include "util/Exception.hpp"
#include "util/config/XCFManager.hpp"

#include "eyeline/sccp/SCCPAddress.hpp"
#include "eyeline/sccp/GlobalTitle.hpp"

#include "eyeline/ss7na/libsccp/types.hpp"
#include "eyeline/ss7na/libsccp/SccpApi.hpp"
#include "eyeline/ss7na/libsccp/SccpApiFactory.hpp"
#include "eyeline/ss7na/libsccp/xcfg/LibSccpCfgReader.hpp"

#include "eyeline/ss7na/libsccp/MessageProperties.hpp"
#include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/N_NOTICE_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/N_PCSTATE_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/N_STATE_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/N_COORD_IND_Message.hpp"

using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using eyeline::ss7na::libsccp::SccpConfig;
using eyeline::ss7na::libsccp::SccpApi;
using eyeline::ss7na::libsccp::SccpApiFactory;
using eyeline::ss7na::libsccp::LibSccpCfgReader;


smsc::logger::Logger *logger;

eyeline::ss7na::libsccp::LibsccpMessage*
parseMessage(eyeline::ss7na::libsccp::MessageInfo& msgInfo)
{
  eyeline::ss7na::common::TP packet(msgInfo.messageType,
                                    msgInfo.msgData.getDataSize(),
                                    msgInfo.msgData.get(),
                                    msgInfo.msgData.getDataSize());
  switch(msgInfo.messageType) {
  case eyeline::ss7na::libsccp::SccpMessageId::N_UNITDATA_IND_MSGCODE: {
    eyeline::ss7na::libsccp::N_UNITDATA_IND_Message* message = new eyeline::ss7na::libsccp::N_UNITDATA_IND_Message();
    message->deserialize(packet);
    return message;
  }
  case eyeline::ss7na::libsccp::SccpMessageId::N_NOTICE_IND_MSGCODE: {
    eyeline::ss7na::libsccp::N_NOTICE_IND_Message* message = new eyeline::ss7na::libsccp::N_NOTICE_IND_Message();
    message->deserialize(packet);
    return message;
  }
  case eyeline::ss7na::libsccp::SccpMessageId::N_PCSTATE_IND_MSGCODE: {
    eyeline::ss7na::libsccp::N_PCSTATE_IND_Message* message = new eyeline::ss7na::libsccp::N_PCSTATE_IND_Message();
    message->deserialize(packet);
    return message;
  }
  case eyeline::ss7na::libsccp::SccpMessageId::N_STATE_IND_MSGCODE: {
    eyeline::ss7na::libsccp::N_STATE_IND_Message* message = new eyeline::ss7na::libsccp::N_STATE_IND_Message();
    message->deserialize(packet);
    return message;
  }
  case eyeline::ss7na::libsccp::SccpMessageId::N_COORD_IND_MSGCODE: {
    eyeline::ss7na::libsccp::N_COORD_IND_Message* message = new eyeline::ss7na::libsccp::N_COORD_IND_Message();
    message->deserialize(packet);
    return message;
  }
  default:
    throw smsc::util::Exception("parseMessage:: got message with unknown type='%u'",
                                msgInfo.messageType);
  }
}
void
bind()
{
  SccpApi & sccpApi = SccpApiFactory::getSccpApiIface();

  uint8_t ssnList[] = { 7, 147 };
  for(unsigned idx = 0; idx < sccpApi.getConnectsCount(); ++idx) {
    sccpApi.connect(idx);
    eyeline::ss7na::libsccp::SccpApi::ErrorCode_e retResult;
    retResult = sccpApi.bind(idx, ssnList, static_cast<uint8_t>(sizeof(ssnList)));
    if ( retResult != eyeline::ss7na::libsccp::SccpApi::OK ) {
      smsc_log_error(logger, "bind failed: err code = %u", retResult);
      exit(1);
    }
  }
}

void
ping(const char* called_addr, const char* calling_addr)
{
  SccpApi & sccpApi = SccpApiFactory::getSccpApiIface();

  eyeline::sccp::SCCPAddress calledAddr(eyeline::sccp::GlobalTitle(0,
                                                                   eyeline::sccp::GlobalTitle::npiISDNTele_e164,
                                                                   eyeline::sccp::GlobalTitle::noaInternational,
                                                                   called_addr),
                                                                   7);

  eyeline::sccp::SCCPAddress callingAddr(eyeline::sccp::GlobalTitle(0,
                                                                    eyeline::sccp::GlobalTitle::npiISDNTele_e164,
                                                                    eyeline::sccp::GlobalTitle::noaInternational,
                                                                    calling_addr),
                                                                    147);

  eyeline::ss7na::libsccp::MessageProperties msgProps;

//  const char* text = "Hello world!";
  uint8_t text[275];
  memset(text, 'A', 252);
  memset(text+252, 'B', 275 - 252);
  msgProps.setHopCount(3);
//  sccpApi.unitdata_req((const uint8_t*)text, static_cast<uint16_t>(strlen(text)),
//                       calledAddr, callingAddr,
//                       msgProps);
  sccpApi.unitdata_req(text, static_cast<uint16_t>(sizeof(text)),
                       calledAddr, callingAddr,
                       msgProps);

  eyeline::ss7na::libsccp::MessageInfo msgInfo;
  if ( sccpApi.msgRecv(&msgInfo) != eyeline::ss7na::libsccp::SccpApi::OK )
    return;

  const eyeline::ss7na::libsccp::LibsccpMessage* msg = parseMessage(msgInfo);
  smsc_log_info(logger, "Got message = '%s'", msg->toString().c_str());
}

void
pong()
{
  SccpApi & sccpApi = SccpApiFactory::getSccpApiIface();

  while(true) {
    eyeline::ss7na::libsccp::MessageInfo msgInfo;
    if ( sccpApi.msgRecv(&msgInfo) != eyeline::ss7na::libsccp::SccpApi::OK )
      return;
    smsc_log_info(logger, "Got message with type=%u", msgInfo.messageType);

    eyeline::ss7na::common::TP packet(msgInfo.messageType,
                                      msgInfo.msgData.getDataSize(),
                                      msgInfo.msgData.get(),
                                      msgInfo.msgData.getDataSize());
    if ( msgInfo.messageType == eyeline::ss7na::libsccp::SccpMessageId::N_UNITDATA_IND_MSGCODE ) {
      eyeline::ss7na::libsccp::N_UNITDATA_IND_Message* request =
          new eyeline::ss7na::libsccp::N_UNITDATA_IND_Message();
      request->deserialize(packet);

      eyeline::ss7na::libsccp::MessageProperties msgProps;
      if ( request->isSetSequenceControl() )
        msgProps.setSequenceControl(request->getSequenceControl());

      sccpApi.unitdata_req(request->getUserData().data, request->getUserData().dataLen,
                           request->getCallingAddress().data, request->getCallingAddress().dataLen,
                           request->getCalledAddress().data, request->getCalledAddress().dataLen,
                           msgProps);
    }
  }
}


int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("init");

  const char* cfgFile;
  if ( argc < 3 ) {
    cfgFile = "config.xml";
    fprintf(stderr, "Usage: %s cfg_file ping_pong_flag called_addr calling_addr. [0 - pong, 1 - ping]\n", argv[0]);
    return 1;
  } else
    cfgFile = argv[1];

  bool makePing = atoi(argv[2]);
  if ( makePing && argc != 5 ) {
    fprintf(stderr, "For ping usage: %s cfg_file 1 called_addr calling_addr.\n", argv[0]);
    return 1;
  }
  try {
    SccpConfig        sccpCfgParms;
    LibSccpCfgReader  cfgReader("libsccp");
    {
      std::auto_ptr<Config> xConfig(XCFManager::getInstance().getConfig(cfgFile)); //throws
      cfgReader.readConfig(*xConfig.get(), sccpCfgParms); //throws
    }

    SccpApiFactory::init();
    SccpApi & sccpApi = SccpApiFactory::getSccpApiIface();

    if (sccpApi.init(sccpCfgParms) != SccpApi::OK) { //throws
      smsc_log_error(logger, "main::: SccpApi initialization failed. Terminated.");
      return 1;
    }

    bind();

    if ( makePing )
      ping(argv[3], argv[4]);
    else
      pong();
  } catch (const std::exception& ex) {
    smsc_log_error(logger, "main::: caught exception [%s]. Terminated.", ex.what());
    return 1;
  } catch (...) {
    smsc_log_error(logger, "main::: caught unexpected exception [...]. Terminated.");
    return 1;
  }

  return 0;
}
