#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_LIBSCCPCONNECTINDICATIONEVENT_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_LIBSCCPCONNECTINDICATIONEVENT_HPP__

# include "logger/Logger.h"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IOEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class LibSccpConnectIndicationEvent : public io_dispatcher::IOEvent {
public:
  LibSccpConnectIndicationEvent(const LinkId& link_id)
  : _acceptedLinkId(link_id), _logger(smsc::logger::Logger::getInstance("sccp_sap"))
  {}

  virtual void handle();
private:
  const LinkId _acceptedLinkId;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
