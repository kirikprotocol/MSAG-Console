#include <utility>

#include "SetOfNotBindedConnections.hpp"
#include "util/Exception.hpp"
#include "IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

SetOfNotBindedConnections::SetOfNotBindedConnections(IOProcessor& io_processor,
                                                     const LinkId& link_set_id)
  : _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _ioProcessor(io_processor), _relatedLinkSetId(link_set_id),
    _positiveBindResponseHasNotBeenGotYet(true),
    _totalLinks(0), _numOfBindFailedLinks(0)
{
  smsc_log_debug(_logger, "SetOfNotBindedConnections::SetOfNotBindedConnections::: this=%p, _relatedLinkSetId='%s'",
                 this, _relatedLinkSetId.toString().c_str());
}

void
SetOfNotBindedConnections::addLink(LinkId link_id)
{
  smsc_log_debug(_logger, "SetOfNotBindedConnections::addLink::: add link with id='%s'",
                 link_id.toString().c_str());
  _notBindedConnections.insert(link_id);
  ++_totalLinks;
}

void
SetOfNotBindedConnections::addLinks(SetOfNotBindedConnections* another_not_binded_conns)
{
  smsc_log_debug(_logger, "SetOfNotBindedConnections::addLinks::: process new links, this=%p, _relatedLinkSetId='%s'",
                 this, _relatedLinkSetId.toString().c_str());
  while (!another_not_binded_conns->_notBindedConnections.empty()) {
    not_binded_conns_t::iterator iter = another_not_binded_conns->_notBindedConnections.begin();
    addLink(*iter);
    another_not_binded_conns->_notBindedConnections.erase(iter);
  }
  smsc_log_debug(_logger, "SetOfNotBindedConnections::addLinks::: new links added, this=%p, _relatedLinkSetId=[%s]",
                   this, _relatedLinkSetId.toString().c_str());
}

LinkId
SetOfNotBindedConnections::getLinkSetId() const
{
  return getRelatedLinkSet()->getLinkId();
}

bool
SetOfNotBindedConnections::commitBindedConnection(const LinkId& link_id_to_smsc)
{
  smsc_log_debug(_logger, "SetOfNotBindedConnections::commitBindedConnection::: try commit bind request for link with id='%s'",
                 link_id_to_smsc.toString().c_str());
  not_binded_conns_t::iterator iter = _notBindedConnections.find(link_id_to_smsc);
  if ( iter == _notBindedConnections.end() )
    return false;

  TimeoutMonitor::timeout_id_t timeoutId = "BND:" + link_id_to_smsc.toString();
  TimeoutMonitor::getInstance().cancelTimeout(timeoutId);

  _notBindedConnections.erase(iter);

  smsc_log_debug(_logger, "SetOfNotBindedConnections::commitBindedConnection::: got positive bind response from link with='%s'",
                 link_id_to_smsc.toString().c_str());

  if ( _positiveBindResponseHasNotBeenGotYet ) {
    _positiveBindResponseHasNotBeenGotYet = false;
    return true;
  } else
    return false;
}

bool
SetOfNotBindedConnections::processFailedBindResponse(const LinkId& link_id_to_smsc)
{
  smsc_log_debug(_logger, "SetOfNotBindedConnections::processFailedBindResponse::: processing of bind failure for link with id='%s'",
                   link_id_to_smsc.toString().c_str());

  not_binded_conns_t::iterator iter = _notBindedConnections.find(link_id_to_smsc);
  if ( iter == _notBindedConnections.end() )
    return false;

  TimeoutMonitor::timeout_id_t timeoutId = "BND:" + link_id_to_smsc.toString();
  TimeoutMonitor::getInstance().cancelTimeout(timeoutId);

  _notBindedConnections.erase(iter);
  if ( !_positiveBindResponseHasNotBeenGotYet ) return false;
  else {
    if ( ++_numOfBindFailedLinks == _totalLinks )
      return true;
    else
      return false;
  }
}

bool
SetOfNotBindedConnections::isEmpty() const
{
  return _notBindedConnections.empty();
}

bool
SetOfNotBindedConnections::isLinkBelongToLinkSet(const LinkId& link_id_to_smsc)
{
  smsc_log_debug(_logger, "SetOfNotBindedConnections::isLinkBelongToLinkSet(linkIdToSmsc=%s), this=%p, _relatedLinkSetId='%s'",
                 link_id_to_smsc.toString().c_str(), this, _relatedLinkSetId.toString().c_str());

  if ( _notBindedConnections.find(link_id_to_smsc) == _notBindedConnections.end() )
    return false;
  else
    return true;
}

LinkSetRefPtr
SetOfNotBindedConnections::getRelatedLinkSet() const
{
  LinkSetRefPtr relatedLinkSet = _ioProcessor.getLinkSet(_relatedLinkSetId);
  if ( !relatedLinkSet.Get() )
    throw smsc::util::Exception("SetOfNotBindedConnections::getRelatedLinkSet::: linkSet with id='%s' is not registered",
                                _relatedLinkSetId.toString().c_str());
  return relatedLinkSet;
}

}}}
