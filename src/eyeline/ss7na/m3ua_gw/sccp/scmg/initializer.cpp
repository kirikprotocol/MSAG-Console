#include "MessagesFactory.hpp"
#include "MessagesInstancer.hpp"
#include "MessagesHandlingDispatcher.hpp"

#include "logger/Logger.h"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

bool registerMessageCreators()
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sccp");
  smsc_log_debug(logger, "enter scmg::registerMessageCreators");
  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::SOGMessage().getMsgCode(),
     new MessagesInstancer<SOGMessage_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::SORMessage().getMsgCode(),
     new MessagesInstancer<SORMessage_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::SSAMessage().getMsgCode(),
     new MessagesInstancer<SSAMessage_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::SSCMessage().getMsgCode(),
     new MessagesInstancer<SSCMessage_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::SSPMessage().getMsgCode(),
     new MessagesInstancer<SSPMessage_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::SSTMessage().getMsgCode(),
     new MessagesInstancer<SSTMessage_HandlingDispatcher>()
    );

  smsc_log_debug(logger, "leave scmg::registerMessageCreators");
  return true;
}

}}}}}
