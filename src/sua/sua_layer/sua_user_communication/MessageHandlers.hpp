#ifndef __SUA_USER_COMMUNICATION_MESSAGEHANDLERS_HPP__
# define __SUA_USER_COMMUNICATION_MESSAGEHANDLERS_HPP__ 1

# include <sua/utilx/Singleton.hpp>
# include <logger/Logger.h>
# include <sua/sua_layer/io_dispatcher/Link.hpp>
# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>

# include <sua/communication/libsua_messages/BindMessage.hpp>
# include <sua/communication/libsua_messages/UnbindMessage.hpp>
# include <sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>
# include <sua/communication/sua_messages/CLDTMessage.hpp>

namespace sua_user_communication {

class MessageHandlers : public utilx::Singleton<MessageHandlers> {
public:
  MessageHandlers();
  void handle(const libsua_messages::BindMessage& message, const communication::LinkId& linkId);
  void handle(const libsua_messages::UnbindMessage& message, const communication::LinkId& linkId);
  void handle(const libsua_messages::N_UNITDATA_REQ_Message& message, const communication::LinkId& linkId);
private:
  sua_messages::CLDTMessage createCLDTMessage(const libsua_messages::N_UNITDATA_REQ_Message& message);

  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
  LinkSetInfoRegistry& _linkSetInfoRegistry;
};

}

#endif
