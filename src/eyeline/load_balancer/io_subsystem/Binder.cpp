#include <utility>

#include "util/Exception.hpp"
#include "Binder.hpp"
#include "IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

Binder::Binder(IOProcessor& io_processor)
  : _ioProcessor(io_processor), _logger(smsc::logger::Logger::getInstance("bindr"))
{}

LinkId
Binder::addSetOfNotBindedConnections(SetOfNotBindedConnections* established_and_not_binded_connections)
{
  smsc::core::synchronization::MutexGuard synchronize(_knownSetsOfNotBindedConnsLock);

  const LinkId& linkSetIdToSmsc = established_and_not_binded_connections->getLinkSetId();
  linksetid_to_notbinded_conns_map_t::iterator iter = _knownSetsOfNotBindedConns.find(linkSetIdToSmsc);

  if ( iter == _knownSetsOfNotBindedConns.end() ) {
    smsc_log_debug(_logger, "Binder::addSetOfNotBindedConnections::: add new SetOfNotBindedConnections instance");
    _knownSetsOfNotBindedConns.insert(std::make_pair(linkSetIdToSmsc,
                                                     established_and_not_binded_connections));
  } else {
    SetOfNotBindedConnections* alreadyAddedNotBindedConnectionsSet = iter->second;
    alreadyAddedNotBindedConnectionsSet->addLinks(established_and_not_binded_connections);
    smsc_log_debug(_logger, "Binder::addSetOfNotBindedConnections::: merge two SetOfNotBindedConnections instances");
    delete established_and_not_binded_connections;
  }
  return linkSetIdToSmsc;
}

bool
Binder::commitBindResponse(const LinkId& link_id_to_smsc)
{
  smsc_log_debug(_logger, "Binder::commitBindResponse::: link to smsc with id='%s' has been binded",
                 link_id_to_smsc.toString().c_str());
  smsc::core::synchronization::MutexGuard synchronize(_knownSetsOfNotBindedConnsLock);
  NotbindedLinksSearchInfo searchRes =
    getSetOfNotBindedConnectionsContainingThisLink(link_id_to_smsc);

  if ( searchRes.notBindedLinks ) {
    bool result = searchRes.notBindedLinks->commitBindedConnection(link_id_to_smsc);
    return result;
  } else
    return false;
}

bool
Binder::processFailedBindResponse(const LinkId& link_id_to_smsc)
{
  smsc_log_debug(_logger, "Binder::processFailedBindResponse::: bind on link to smsc with id='%s' has been failed",
                 link_id_to_smsc.toString().c_str());

  smsc::core::synchronization::MutexGuard synchronize(_knownSetsOfNotBindedConnsLock);
  NotbindedLinksSearchInfo searchRes =
    getSetOfNotBindedConnectionsContainingThisLink(link_id_to_smsc);

  if ( searchRes.notBindedLinks ) {
    bool result = searchRes.notBindedLinks->processFailedBindResponse(link_id_to_smsc);
    return result;
  } else
    return false;
}

Binder::NotbindedLinksSearchInfo
Binder::getSetOfNotBindedConnectionsContainingThisLink(const LinkId& link_id_to_smsc)
{
  for (linksetid_to_notbinded_conns_map_t::iterator iter = _knownSetsOfNotBindedConns.begin(), end_iter = _knownSetsOfNotBindedConns.end();
      iter != end_iter; ++iter) {
    if ( iter->second->isLinkBelongToLinkSet(link_id_to_smsc) )
      return NotbindedLinksSearchInfo(iter->second, iter);
  }
  return NotbindedLinksSearchInfo();
}

void
Binder::removeBindingInfo(const LinkId& link_set_id_to_smsc)
{
  smsc::core::synchronization::MutexGuard synchronize(_knownSetsOfNotBindedConnsLock);
  linksetid_to_notbinded_conns_map_t::iterator iter =
    _knownSetsOfNotBindedConns.find(link_set_id_to_smsc);

  if ( iter == _knownSetsOfNotBindedConns.end() )
    return;

  smsc_log_debug(_logger, "Binder::removeBindingInfo(link_set_id_to_smsc='%s'), delete SetOfNotBindedConnections=%p",
                 link_set_id_to_smsc.toString().c_str(), iter->second);

  delete iter->second;
  _knownSetsOfNotBindedConns.erase(iter);
}

}}}
