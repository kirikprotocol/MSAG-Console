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
# include <sua/communication/libsua_messages/MErrorMessage.hpp>
# include <sua/communication/libsua_messages/EncapsulatedSuaMessage.hpp>

namespace sua_user_communication {

class MessageHandlers : public utilx::Singleton<MessageHandlers> {
public:
  MessageHandlers();
  void handle(const libsua_messages::BindMessage& message, const communication::LinkId& linkId);
  void handle(const libsua_messages::UnbindMessage& message, const communication::LinkId& linkId);
  void handle(const libsua_messages::MErrorMessage& message, const communication::LinkId& linkId);
  void handle(const libsua_messages::EncapsulatedSuaMessage& message, const communication::LinkId& linkId);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
  LinkSetInfoRegistry& _linkSetInfoRegistry;
};

}

#endif
