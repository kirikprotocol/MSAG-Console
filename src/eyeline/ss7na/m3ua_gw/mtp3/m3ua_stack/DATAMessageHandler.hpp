#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_DATAMESSAGEHANDLER_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_DATAMESSAGEHANDLER_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DATAMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

class DATAMessageHandler {
public:
  DATAMessageHandler();
  void handle(const messages::DATAMessage& message, const common::LinkId& link_id);
private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

}}}}}

#endif
