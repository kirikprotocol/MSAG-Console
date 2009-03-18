#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_MESSAGEHANDLERS_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_MESSAGEHANDLERS_HPP__

# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/Link.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>

# include <eyeline/sua/communication/libsua_messages/BindMessage.hpp>
# include <eyeline/sua/communication/libsua_messages/UnbindMessage.hpp>
# include <eyeline/sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>
# include <eyeline/sua/communication/sua_messages/CLDTMessage.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

class MessageHandlers : public utilx::Singleton<MessageHandlers> {
public:
  MessageHandlers();
  void handle(const communication::libsua_messages::BindMessage& message, const communication::LinkId& linkId);
  void handle(const communication::libsua_messages::UnbindMessage& message, const communication::LinkId& linkId);
  void handle(const communication::libsua_messages::N_UNITDATA_REQ_Message& message, const communication::LinkId& linkId);
private:
  communication::sua_messages::CLDTMessage createCLDTMessage(const communication::libsua_messages::N_UNITDATA_REQ_Message& message);

  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
  LinkSetInfoRegistry& _linkSetInfoRegistry;
};

}}}}

#endif
