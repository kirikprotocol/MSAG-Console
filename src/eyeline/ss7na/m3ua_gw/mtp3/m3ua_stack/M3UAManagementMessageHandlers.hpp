#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_M3UAMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_M3UAMANAGEMENTMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/ErrorMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/NotifyMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

class M3UAManagementMessageHandlers {
public:
  M3UAManagementMessageHandlers();
  void handle(const messages::ErrorMessage& message, const common::LinkId& link_id);
  void handle(const messages::NotifyMessage& message, const common::LinkId& link_id);
private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

}}}}}

#endif
