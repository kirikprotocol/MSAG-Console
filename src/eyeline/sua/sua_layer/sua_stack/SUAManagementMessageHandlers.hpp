#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_SUAMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_SUAMANAGEMENTMESSAGEHANDLERS_HPP__

# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>

# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
# include <eyeline/sua/communication/sua_messages/ErrorMessage.hpp>
# include <eyeline/sua/communication/sua_messages/NotifyMessage.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class SUAManagementMessageHandlers
  : public utilx::Singleton<SUAManagementMessageHandlers> {
public:
  SUAManagementMessageHandlers();
  void handle(const communication::sua_messages::ErrorMessage& message, const communication::LinkId& linkId);
  void handle(const communication::sua_messages::NotifyMessage& message, const communication::LinkId& linkId);
private:
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

}}}}

#endif
