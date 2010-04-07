#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_CLCOMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_CLCOMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDTMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDRMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class CLCOMessageHandlers : public utilx::Singleton<CLCOMessageHandlers> {
public:
  CLCOMessageHandlers();
  void handle(const messages::CLDTMessage& message,
              const common::LinkId& link_id);

  void handle(const messages::CLDRMessage& message,
              const common::LinkId& link_id);
private:
  void prepareAndSendCLDRMessage(const messages::CLDTMessage& cldt_message,
                                 const common::LinkId& link_id,
                                 common::return_cause_value_t return_cause);
  void sendIndicationPrimitive(const messages::CLDTMessage& cldt_message,
                               const common::LinkId& out_linkset_id);
  void forwardMessageToSGP(const messages::CLDTMessage& cldt_message,
                           const common::LinkId& link_id);

  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

}}}}

#endif
