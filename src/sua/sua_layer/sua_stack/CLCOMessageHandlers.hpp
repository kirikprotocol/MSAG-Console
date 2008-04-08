#ifndef __SUA_STACK_COMMUNICATION_CLCOMESSAGEHANDLERS_HPP__
# define __SUA_STACK_COMMUNICATION_CLCOMESSAGEHANDLERS_HPP__ 1

# include <logger/Logger.h>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/communication/sua_messages/CLDTMessage.hpp>
# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

namespace sua_stack {

class CLCOMessageHandlers : public utilx::Singleton<CLCOMessageHandlers> {
public:
  CLCOMessageHandlers();
  void handle(const sua_messages::CLDTMessage& message, const communication::LinkId& linkId);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

}

#endif
