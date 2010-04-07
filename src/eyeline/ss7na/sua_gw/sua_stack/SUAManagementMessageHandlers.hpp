#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_SUAMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_SUAMANAGEMENTMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/ErrorMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/NotifyMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class SUAManagementMessageHandlers
  : public utilx::Singleton<SUAManagementMessageHandlers> {
public:
  SUAManagementMessageHandlers();
  void handle(const messages::ErrorMessage& message, const common::LinkId& link_id);
  void handle(const messages::NotifyMessage& message, const common::LinkId& link_id);
private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

}}}}

#endif
