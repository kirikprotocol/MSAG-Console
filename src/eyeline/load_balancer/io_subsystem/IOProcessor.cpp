#include <utility>

#include "util/Exception.hpp"
#include "IOProcessor.hpp"
#include "SetOfFailedConnections.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
IOProcessor::startup()
{
  smsc_log_debug(_logger, "Start IOProcessor instance with id=%d", getId());
  _eventProcessorsPool.startup();
  _packetReader.Start();
  _packetWriter.Start();
  _reconnector.Start();
  smsc_log_debug(_logger, "IOProcessor instance with id=%d has been started", getId());
}

void
IOProcessor::shutdown()
{
  smsc_log_debug(_logger, "Try shutdown IOProcessor instance with id=%d", getId());
  _reconnector.shutdown();
  _packetReader.shutdown();
  _packetWriter.shutdown();
  _eventProcessorsPool.shutdown();
  smsc_log_debug(_logger, "IOProcessor with id=%d has been stopped", getId());
}

LinkId
IOProcessor::addLink(LinkRefPtr& link)
{
  //std::auto_ptr<Link> linkPtrGuard(link);
  smsc_log_debug(_logger, "IOProcessor::addLink::: add link with id='%s'",
                 link->getLinkId().toString().c_str());
  smsc::core::synchronization::MutexGuard synchronize(_activeLinksLock);

  if ( _numOfActiveLinks >= _maxLinksPerProcessor )
    throw smsc::util::Exception("IOProcessor::addLink::: upper limit (=%d) of links per processor was achieved",
                                _maxLinksPerProcessor);
  if ( _activeLinks.find(link->getLinkId()) != _activeLinks.end() )
    throw smsc::util::Exception("IOProcessor::addLink::: link with id='%s' has been already registered",
                                    link->getLinkId().toString().c_str());

  _packetReader.addInputStream(link->getLinkId());
  _packetWriter.addOutputStream(link->getLinkId());
  link->assignPacketWriter(&_packetWriter);

  std::pair<active_links_t::iterator, bool> ins_res =
    _activeLinks.insert(std::make_pair(link->getLinkId(), link));
  ++_numOfActiveLinks;

  return link->getLinkId();
}

LinkId
IOProcessor::addLinkSet(LinkSetRefPtr& link_set)
{
  smsc_log_debug(_logger, "IOProcessor::addLinkSet::: add linkset with id='%s'",
                 link_set->getLinkId().toString().c_str());

  smsc::core::synchronization::MutexGuard synchronize(_activeLinkSetsLock);

  std::pair<active_linksets_t::iterator, bool> ins_res =
    _activeLinkSets.insert(std::make_pair(link_set->getLinkId(), link_set));

  if ( !ins_res.second )
    throw smsc::util::Exception("IOProcessor::addLinkSet::: linkset with id='%s' has been already registered",
                                link_set->getLinkId().toString().c_str());

  return link_set->getLinkId();
}

LinkRefPtr
IOProcessor::getLink(const LinkId& link_id) const
{
  smsc::core::synchronization::MutexGuard synchronize(_activeLinksLock);

  smsc_log_debug(_logger, "IOProcessor::getLink::: try get Link for id='%s'",
                 link_id.toString().c_str());

  active_links_t::const_iterator iter = _activeLinks.find(link_id);
  if ( iter == _activeLinks.end() ) {
    smsc_log_error(_logger, "IOProcessor::getLink::: Link for id='%s' doesn't exist",
                   link_id.toString().c_str());
    return LinkRefPtr(NULL);
  } else
    return iter->second;
}

LinkSetRefPtr
IOProcessor::getLinkSet(const LinkId& link_set_id) const
{
  smsc::core::synchronization::MutexGuard synchronize(_activeLinkSetsLock);

  smsc_log_debug(_logger, "IOProcessor::getLinkSet::: try get LinkSet for id='%s'",
                 link_set_id.toString().c_str());

  active_linksets_t::const_iterator iter = _activeLinkSets.find(link_set_id);
  if ( iter == _activeLinkSets.end() ) {
    smsc_log_error(_logger, "IOProcessor::getLinkSet::: LinkSet for id='%s' doesn't exist",
                   link_set_id.toString().c_str());
    return LinkSetRefPtr(NULL);
  } else
    return iter->second;
}

void
IOProcessor::terminateConnection(const LinkId& link_id,
                                 LinkRefPtr* terminated_link_ref)
{
  smsc_log_error(_logger, "IOProcessor::terminateConnection::: try terminate Link with id='%s'",
                 link_id.toString().c_str());
  LinkRefPtr removingLink; // to delay object destruction until the end of scope this variable
  {
    smsc::core::synchronization::MutexGuard synchronize(_activeLinksLock);

    active_links_t::iterator iter = _activeLinks.find(link_id);
    if ( iter == _activeLinks.end() ) {
      smsc_log_debug(_logger, "IOProcessor::terminateConnection::: Link with id='%s' doesn't exist",
                     link_id.toString().c_str());
      return;
    }
    removingLink = iter->second;
    _activeLinks.erase(iter);
    --_numOfActiveLinks;
  }
  if ( terminated_link_ref )
    *terminated_link_ref = removingLink;

  _packetReader.removeInputStream(link_id);
  _packetWriter.removeOutputStream(link_id);

  smsc_log_error(_logger, "IOProcessor::terminateConnection::: Link with id='%s' has been terminated",
                 link_id.toString().c_str());
}

void
IOProcessor::removeIncomingLink(const LinkId& link_id)
{
  terminateConnection(link_id);
}

LinkId
IOProcessor::removeOutcomingLink(const LinkId& link_id, bool no_need_reconnect)
{
  smsc_log_debug(_logger, "IOProcessor::removeOutcomingLink::: try remove out Link with id='%s', no_need_reconnect flag=%d",
                 link_id.toString().c_str(), no_need_reconnect);

  LinkId linkSetId = getLinkSetIdOwnerForThisLink(link_id);
  LinkRefPtr terminatedLink;
  terminateConnection(link_id, &terminatedLink);

  TimeoutMonitor::timeout_id_t timeoutId = "BND:" + link_id.toString();
  TimeoutMonitor::getInstance().cancelTimeout(timeoutId);
  timeoutId = "UNBND:" + link_id.toString();
  TimeoutMonitor::getInstance().cancelTimeout(timeoutId);

  LinkId emptyLinkId;
  if ( linkSetId != emptyLinkId ) {
    smsc_log_debug(_logger, "IOProcessor::removeOutcomingLink::: for Link with id='%s' related linkSetId='%s'",
                   link_id.toString().c_str(), linkSetId.toString().c_str());

    LinkSetRefPtr linkSet = getLinkSet(linkSetId);
    if ( linkSet.Get() ) {
      if ( !linkSet->isEmpty() ) {
        if ( linkSet->removeLink(link_id) )
          // there isn't no more link in linkset
          return linkSetId;
      }
      if ( !no_need_reconnect )
        scheduleConnectionForRestoring(linkSet->getLinkId(), terminatedLink.Get());
    }
  }
  return LinkId();
}

void
IOProcessor::scheduleConnectionForRestoring(const LinkId& linkset_id,
                                            Link* terminated_link)
{
  smsc_log_debug(_logger, "IOProcessor::scheduleConnectionForRestoring::: schedule connection restoring attempt, linksetId='%s'",
                 linkset_id.toString().c_str());
  SetOfFailedConnections* failedConnsSet =
    new SetOfFailedConnections(*this, linkset_id);
  failedConnsSet->addLink(LinkRefPtr(terminated_link->createNewOutLink()));
  _reconnector.addFailedConnections(failedConnsSet);
}

void
IOProcessor::removeLinkSet(const LinkId& link_set_id)
{
  smsc_log_debug(_logger, "IOProcessor::removeLinkSet::: try remove linkset with id='%s'",
                 link_set_id.toString().c_str());

  LinkSetRefPtr linkSet;
  {
    smsc::core::synchronization::MutexGuard synchronize(_activeLinkSetsLock);

    active_linksets_t::iterator iter = _activeLinkSets.find(link_set_id);
    if ( iter == _activeLinkSets.end() )
      return;
    linkSet = iter->second;
    _activeLinkSets.erase(iter);
  }
  LinkSet::LinkSetIterator linkSetIterator = linkSet->getIterator();
  while ( linkSetIterator.hasElement() ) {
    removeOutcomingLink(linkSetIterator.getCurrentElement().getLinkId(), true);
    linkSetIterator.next();
  }
  _reconnector.cancelReconnectionAttempts(link_set_id);
  smsc_log_debug(_logger, "IOProcessor::removeLinkSet::: linkset with id='%s' has been removed",
                   link_set_id.toString().c_str());
}

void
IOProcessor::cleanUpConnection(const LinkId& link_id)
{
  smsc_log_debug(_logger, "IOProcessor::cleanUPConnection::: try cleanup link with id='%s'",
                   link_id.toString().c_str());
  _switchCircuitCtrl.deactivateConnection(link_id, *this);
}

bool
IOProcessor::canProcessNewSocket() const
{
  smsc_log_debug(_logger, "IOProcessor::canProcessNewSocket::: _numOfActiveLinks=%d, _maxLinksPerProcessor=%d",
                 _numOfActiveLinks, _maxLinksPerProcessor);
  smsc::core::synchronization::MutexGuard synchronize(_activeLinksLock);
  return _numOfActiveLinks < _maxLinksPerProcessor;
}

LinkId
IOProcessor::getLinkSetIdOwnerForThisLink(const LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_activeLinksLock);
  active_links_t::iterator iter = _activeLinks.find(link_id);
  if ( iter == _activeLinks.end() )
    return LinkId();
  else
    return iter->second->getRelatedLinkSetId();
}

}}}
