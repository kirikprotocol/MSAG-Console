#include "initializer.hpp"
#include "MessagesInstancer.hpp"
#include "MessagesHandlingDispatcher.hpp"
#include "MessagesFactory.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

bool registerMessageCreators()
{
  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::UDT().getMsgCode(),
     new MessagesInstancer<UDT_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::UDTS().getMsgCode(),
     new MessagesInstancer<UDTS_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::XUDT().getMsgCode(),
     new MessagesInstancer<XUDT_HandlingDispatcher>()
    );

  MessagesFactory::getInstance().registerMessageCreator
    (
     messages::XUDTS().getMsgCode(),
     new MessagesInstancer<XUDTS_HandlingDispatcher>()
    );

  return true;
}

}}}}
