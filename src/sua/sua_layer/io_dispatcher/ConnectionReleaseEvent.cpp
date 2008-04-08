#include "ConnectionReleaseEvent.hpp"

namespace io_dispatcher {

ConnectionReleaseEvent::ConnectionReleaseEvent(const communication::LinkId& releasedLinkId)
  : _releasedLinkId(releasedLinkId), _logger(smsc::logger::Logger::getInstance("io_dsptch")), _cMgr(ConnectMgr::getInstance())
{}

void
ConnectionReleaseEvent::handle()
{
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(_releasedLinkId), _releasedLinkId);
  //  delete _cMgr.removeLink(_releasedLinkId);
  smsc_log_info(_logger, "ConnectionReleaseEvent::handle::: connection with id=[%s] has been closed", _releasedLinkId.getValue().c_str());
}

}
