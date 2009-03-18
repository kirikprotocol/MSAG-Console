#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_CLCOMESSAGEHANDLERS_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_CLCOMESSAGEHANDLERS_HPP__

# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/communication/sua_messages/CLDTMessage.hpp>
# include <eyeline/sua/communication/sua_messages/CLDRMessage.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class CLCOMessageHandlers : public utilx::Singleton<CLCOMessageHandlers> {
public:
  CLCOMessageHandlers();
  void handle(const communication::sua_messages::CLDTMessage& message,
              const communication::LinkId& linkId);

  void handle(const communication::sua_messages::CLDRMessage& message,
              const communication::LinkId& linkId);
private:
  void prepareAndSendCLDRMessage(const communication::sua_messages::CLDTMessage& cldtMessage,
                                 const communication::LinkId& linkId,
                                 communication::return_cause_value_t returnCause);

  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
};

}}}}

#endif
