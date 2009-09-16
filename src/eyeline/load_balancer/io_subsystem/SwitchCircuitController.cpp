# include <memory>

# include "eyeline/corex/io/IOExceptions.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/io_subsystem/types.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

SwitchCircuitController::SwitchCircuitController(const std::string& link_set_prefix,
                                                 smsc::util::config::ConfigView& out_links_cfg_section,
                                                 SwitchingTable& switching_table)
: _switchTable(switching_table), _linkSetPrefix(link_set_prefix),
  _logger(smsc::logger::Logger::getInstance("io_subsystem")), _totalNumberOfOutLinks(0)
{
  std::auto_ptr<std::set<std::string> > setGuard(out_links_cfg_section.getShortSectionNames());
  for(std::set<std::string>::iterator iter = setGuard->begin(), end_iter = setGuard->end();
      iter != end_iter; ++iter) {
    std::auto_ptr<smsc::util::config::ConfigView> outConnectDefSection(out_links_cfg_section.getSubConfig((*iter).c_str()));

    addOutConnectDefinition(outConnectDefSection->getInt("link_index"),
                            *iter,
                            outConnectDefSection->getString("peer_host"),
                            outConnectDefSection->getInt("peer_port"),
                            outConnectDefSection->getInt("connect_timeout"),
                            outConnectDefSection->getInt("bind_resp_wait_timeout"),
                            outConnectDefSection->getInt("unbind_resp_wait_timeout"));
    ++_totalNumberOfOutLinks;
  }
  smsc_log_info(_logger, "SwitchCircuitController::SwitchCircuitController::: switching configuration was set: _linkSetPrefix=%s,_totalNumberOfOutLinks=%d",
                _linkSetPrefix.c_str(), _totalNumberOfOutLinks);
}

void
SwitchCircuitController::activateConnection(corex::io::network::TCPSocket* new_socket,
                                            IOProcessor& io_processor)
{
  smsc_log_info(_logger, "SwitchCircuitController::activateConnection::: try activate outcoming connections for incoming connection=[%s], IOProcessor's id=%d",
                new_socket->toString().c_str(), io_processor.getId());
  LinkSetRefPtr linkSet(createLinkSet());
  smsc_log_debug(_logger, "SwitchCircuitController::activateConnection::: linkSet with id='%s' has been created",
                 linkSet->getLinkId().toString().c_str());
  std::auto_ptr<SetOfFailedConnections> failedConns;
  std::auto_ptr<SetOfNotBindedConnections> notBindedConns;
  for(known_links_t::const_iterator iter = _knownLinks.begin(), end_iter = _knownLinks.end();
      iter != end_iter; ++iter) {
    LinkRefPtr link(createOutcomingLink((*iter)._linkIndex,
                                        (*iter)._peerHost, (*iter)._peerPort,
                                        (*iter)._connectTimeout, (*iter)._bindRespWaitTimeout,
                                        (*iter)._unbindRespWaitTimeout));
    try {
      smsc_log_debug(_logger, "SwitchCircuitController::activateConnection::: try connect to '%s:%d'",
                     (*iter)._peerHost.c_str(), (*iter)._peerPort);

      link->establish();
      smsc_log_debug(_logger, "SwitchCircuitController::activateConnection::: connection to '%s:%d' has been established",
                     (*iter)._peerHost.c_str(), (*iter)._peerPort);

      if ( !notBindedConns.get() )
        notBindedConns.reset(new SetOfNotBindedConnections(io_processor, linkSet->getLinkId()));
      notBindedConns->addLink(link->getLinkId());
      io_processor.addLink(link);
      linkSet->addLink(link);
    } catch (corex::io::ConnectionFailedException& ex) {
      smsc_log_debug(_logger, "SwitchCircuitController::activateConnection::: can't connect to '%s:%d'",
                     (*iter)._peerHost.c_str(), (*iter)._peerPort);

      if ( !failedConns.get() )
        failedConns.reset(new SetOfFailedConnections(io_processor, linkSet->getLinkId()));
      failedConns->addLink(link);
    }
  }
  if ( !notBindedConns.get() ) {
    delete new_socket;
  } else {
    io_processor.addLinkSet(linkSet);
    io_processor.getBinder().addSetOfNotBindedConnections(notBindedConns.release());

    if ( failedConns.get() )
      io_processor.getReconnector().addFailedConnections(failedConns.release());

    LinkRefPtr newIncomingLink(createIncomingLink(new_socket));
    _switchTable.insertSwitching(newIncomingLink->getLinkId(),
                                 linkSet->getLinkId());
    io_processor.addLink(newIncomingLink);
    smsc_log_info(_logger, "SwitchCircuitController::activateConnection::: out connections has been established for incoming connection=[%s], IOProcessor's id=%d",
                  new_socket->toString().c_str(), io_processor.getId());
  }
}

void
SwitchCircuitController::deactivateConnection(const LinkId& link_id,
                                              IOProcessor& io_processor)
{
  LinkId linkSetOwnerId;
  LinkId linkSetId;

  if ( _switchTable.getSwitching(link_id, &linkSetId) ) {
    io_processor.removeIncomingLink(link_id);
    _switchTable.removeSwitching(link_id, &linkSetId);
    io_processor.removeLinkSet(linkSetId);
  } else {
    linkSetOwnerId = io_processor.getLinkSetIdOwnerForThisLink(link_id);
    // in this case link_id identify a link to smsc
    linkSetId = io_processor.removeOutcomingLink(link_id);
    LinkId emptyLinkId;
    if ( linkSetId != emptyLinkId ) {
      LinkId smeLinkId;
      io_processor.removeLinkSet(linkSetId);
      if ( _switchTable.getSwitching(linkSetId, &smeLinkId) ) {
        io_processor.removeIncomingLink(smeLinkId);
        _switchTable.removeSwitching(smeLinkId);
      }
    }
  }
}

void
SwitchCircuitController::addOutConnectDefinition(unsigned int link_index,
                                                 const std::string& link_name,
                                                 const char* peer_host, unsigned peer_port,
                                                 unsigned int connect_timeout,
                                                 unsigned int bind_resp_wait_timeout,
                                                 unsigned int unbind_resp_wait_timeout)
{
  if ( _knownLinkIds.find(link_name) == _knownLinkIds.end() ) {
    smsc_log_info(_logger, "SwitchCircuitController::addOutConnectDefinition::: add new out connect definition: link id=%s,peer_host=%s,peer_port=%d,connect_timeout=%d,unbind_resp_wait_timeout=%d",
                  link_name.c_str(),peer_host,peer_port,connect_timeout,unbind_resp_wait_timeout);
    _knownLinks.push_back(OutLinkDefinition(link_index,
                                            link_name,
                                            peer_host,
                                            peer_port,
                                            connect_timeout,
                                            bind_resp_wait_timeout,
                                            unbind_resp_wait_timeout));
    _knownLinkIds.insert(link_name);
  } else
    smsc_log_error(_logger, "link with id %s already exists, skip it", link_name.c_str());
}

const std::string&
SwitchCircuitController::getLinkSetPrefix() const
{
  return _linkSetPrefix;
}

unsigned
SwitchCircuitController::getTotalNumberOfOutLinks() const
{
  return _totalNumberOfOutLinks;
}

}}}
