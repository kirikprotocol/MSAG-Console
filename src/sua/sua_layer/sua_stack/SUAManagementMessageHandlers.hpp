#ifndef __SUA_STACK_COMMUNICATION_SUAMANAGEMENTMESSAGEHANDLERS_HPP__
# define __SUA_STACK_COMMUNICATION_SUAMANAGEMENTMESSAGEHANDLERS_HPP__ 1

# include <sua/utilx/Singleton.hpp>
# include <logger/Logger.h>
# include <sua/communication/LinkId.hpp>

# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

# include <sua/communication/sua_messages/ErrorMessage.hpp>
# include <sua/communication/sua_messages/NotifyMessage.hpp>

namespace sua_stack {

class SUAManagementMessageHandlers
  : public utilx::Singleton<SUAManagementMessageHandlers> {
public:
  SUAManagementMessageHandlers();
  void handle(const sua_messages::ErrorMessage& message, const communication::LinkId& linkId);
  void handle(const sua_messages::NotifyMessage& message, const communication::LinkId& linkId);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

}

#endif

