#ifndef __EYELINE_SS7NA_M3UAGW_IODISPATCHER_CONNECTIONCLOSEDEVENT_HPP__
# define __EYELINE_SS7NA_M3UAGW_IODISPATCHER_CONNECTIONCLOSEDEVENT_HPP__

# include "logger/Logger.h"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IOEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace io_dispatcher {

class ConnectionClosedEvent : public common::io_dispatcher::IOEvent {
public:
  ConnectionClosedEvent(const common::io_dispatcher::LinkPtr& closed_link)
  : _closedLink(closed_link), _logger(smsc::logger::Logger::getInstance("io_disp"))
  {}

  virtual void handle();

private:
  common::io_dispatcher::LinkPtr _closedLink;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
