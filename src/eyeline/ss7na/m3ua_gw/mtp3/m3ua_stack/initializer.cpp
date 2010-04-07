#include "initializer.hpp"
#include "eyeline/ss7na/m3ua_gw/types.hpp"
#include "eyeline/ss7na/common/io_dispatcher/MessagesFactory.hpp"
#include "eyeline/ss7na/common/AdaptationLayer_MsgCodesIndexer.hpp"
#include "eyeline/ss7na/common/io_dispatcher/MessagesInstancer.hpp"

#include "MessagesHandlingDispatcher.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

bool registerMessageCreators()
{
  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::ActiveAckMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<ActiveAckMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::ActiveMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<ActiveMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DATAMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DATAMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DAVAMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DAVAMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DRSTMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DRSTMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DUNAMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DUNAMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DUPUMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DUPUMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DAUDMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DAUDMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::SCONMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<SCONMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DownAckMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DownAckMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::DownMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<DownMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::ErrorMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<ErrorMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::InactiveAckMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<InactiveAckMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::InactiveMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<InactiveMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::NotifyMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<NotifyMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::UPAckMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<UPAckMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     common::AdaptationLayer_MsgCodesIndexer::getMessageIndex(messages::UPMessage().getMsgCode()),
     new common::io_dispatcher::MessagesInstancer<UPMessage_HandlingDispatcher>(),
     PROTOCOL_M3UA
     );

  return true;
}

}}}}}
