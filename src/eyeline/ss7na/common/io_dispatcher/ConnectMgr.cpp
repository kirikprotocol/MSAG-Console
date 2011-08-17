#include <utility>
#include <assert.h>

#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/corex/io/network/Socket.hpp"
#include "eyeline/utilx/PreallocatedMemoryManager.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "eyeline/ss7na/common/types.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"

#include "GotMessageEvent.hpp"
#include "Connection.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

void
ConnectMgr::addConnectAcceptor(const std::string& acceptor_name, ConnectAcceptor* connect_acceptor)
{
  {
    smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);
    smsc_log_info(_logger, "ConnectMgr::addConnectAcceptor::: add ConnectAcceptor: acceptorName=[%s], corresponding server socket=[%s]",
                  acceptor_name.c_str(), connect_acceptor->getServerSocket()->toString().c_str());

    _connectAcceptors.insert(std::make_pair(acceptor_name, connect_acceptor));
    _socket2connectAcceptor.insert(std::make_pair(connect_acceptor->getServerSocket()->toString(), connect_acceptor));
  }

  _ioObjectsPool.insert(connect_acceptor->getServerSocket());
}

ConnectAcceptor*
ConnectMgr::removeConnectAcceptor(const std::string& acceptor_name, bool close_all_established_connects)
{
  ConnectAcceptor* result=NULL;
  smsc_log_debug(_logger, "ConnectMgr::removeConnectAcceptor::: try remove ConnectAcceptor with acceptorName=[%s]", acceptor_name.c_str());
  {
    smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);

    connect_acceptors_map_t::iterator iter = _connectAcceptors.find(acceptor_name);
    if ( iter != _connectAcceptors.end() ) {
      result=iter->second;
      _socket2connectAcceptor.erase(result->getServerSocket()->toString());

      if ( close_all_established_connects ) {
        accptrs_to_connects_t::iterator accptr2connIter = _acceptor2establishedConnects.find(acceptor_name);
        if ( accptr2connIter != _acceptor2establishedConnects.end() ) {
          std::set<LinkId>* acceptedConnections = accptr2connIter->second;
          while ( !acceptedConnections->empty() ) {
            std::set<LinkId>::iterator set_iter = acceptedConnections->begin();
            LinkId linkId = *set_iter;
            acceptedConnections->erase(set_iter);
            removeLink(linkId, false);
          }
          delete acceptedConnections;
        }
      }
      _connectAcceptors.erase(iter);
      smsc_log_info(_logger, "ConnectMgr::removeConnectAcceptor::: ConnectAcceptor with acceptorName=[%s] has been removed; corresponding server socket=[%s]", acceptor_name.c_str(), result->getServerSocket()->toString().c_str());
    }
  }

  if ( result )
    _ioObjectsPool.remove(result->getServerSocket());

  return result;
}

void
ConnectMgr::addLink(const LinkId& link_id, Link* link)
{
  smsc_log_debug(_logger, "ConnectMgr::addLink::: try add link=[%s]", link_id.getValue().c_str());
  addLink(link_id, LinkPtr(link));
}

void
ConnectMgr::addLink(const LinkId& link_id, const LinkPtr& link)
{
  {
    smsc::core::synchronization::MutexGuard guard(_linkLock);
    _activeLinks.insert(std::make_pair(link_id, link));
  }

  _ioObjectsPool.insert(link->getSocket()->getInputStream());

  {
    smsc::core::synchronization::MutexGuard guard(_sock2link_lock);
    _socket2link.insert(std::make_pair(link->getSocket()->toString(), link));
  }

  smsc_log_info(_logger, "ConnectMgr::addLink::: link=[%s] has been added", link->getLinkId().getValue().c_str());
}

LinkPtr
ConnectMgr::removeLink(const LinkId& link_id, bool cleanup_acceptor_established_conns /* =true */)
{
  smsc_log_debug(_logger, "ConnectMgr::removeLink::: try remove link=[%s]", link_id.getValue().c_str());
  LinkPtr curConnect;
  {
    smsc::core::synchronization::MutexGuard guard(_linkLock);
    links_t::iterator iter = _activeLinks.find(link_id);
    if ( iter != _activeLinks.end() ) {
      curConnect = iter->second;
      _activeLinks.erase(iter);
    }
  }
  if ( curConnect.Get() != NULL ) {
    const ConnectAcceptor* connectCreator = curConnect->getCreator();
    if ( cleanup_acceptor_established_conns && connectCreator ) {
      smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);
      accptrs_to_connects_t::iterator accptr2connIter = _acceptor2establishedConnects.find(connectCreator->getName());
      if ( accptr2connIter != _acceptor2establishedConnects.end() ) {
        std::set<LinkId> *links = accptr2connIter->second;
        links->erase(link_id);
      }
    }
    {
      smsc::core::synchronization::MutexGuard guard(_sock2link_lock);
      _socket2link.erase(curConnect->getSocket()->toString());
    }

    _ioObjectsPool.remove(curConnect->getSocket()->getInputStream());
    smsc_log_info(_logger, "ConnectMgr::removeLink::: link=[%s] has been removed", curConnect->getLinkId().getValue().c_str());
    return curConnect;
  } else
    return LinkPtr(NULL);
}

bool
ConnectMgr::existLink(const LinkId& link_id) const
{
  smsc::core::synchronization::MutexGuard guard(_linkLock);
  return _activeLinks.find(link_id) != _activeLinks.end();
}

ConnectMgr::ConnectMgr()
  : _ioObjectsPool(MAX_SOCKET_POOL_SIZE), _listenStatus(corex::io::IOObjectsPool::OK_NO_EVENTS),
    _logger(smsc::logger::Logger::getInstance("io_dsptch")), _shutdownInProgress(false)
{}

IOEvent*
ConnectMgr::readPacketAndMakeEvent(LinkPtr& cur_connect, bool fill_up_input_buffer /*=false*/)
{
  if ( fill_up_input_buffer )
    cur_connect->bufferInputTransportPackets(); // read data of max possible size into buffer.

  if ( cur_connect->hasReadyTransportPacket() ) // check if buffered data contains the whole transport packet
  {
    smsc_log_debug(_logger, "ConnectMgr::readPacketAndMakeEvent::: call receive() for connect=[%s]", cur_connect->getLinkId().getValue().c_str());
    TP* transportPacket = cur_connect->receive(); // return pointer to TP from preallocated memory
    utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_EVENT>();
    smsc_log_debug(_logger, "ConnectMgr::readPacketAndMakeEvent::: new packet has been read from connect=[%s], packetType=%u, protocolClass=%u, packetLen=%u", cur_connect->getLinkId().getValue().c_str(), transportPacket->packetType, transportPacket->protocolClass, transportPacket->packetLen);

    return new (ptr_desc->allocated_memory) GotMessageEvent(transportPacket, cur_connect->getLinkId());
  }
  return NULL;
}

IOEvent*
ConnectMgr::cleanupLinkInfo(const LinkId& link_id)
{
  removeLinkFromLinkSets(getLinkSetIds(link_id), link_id);

  LinkPtr closedLink = removeLink(link_id);
  _curConnect = NULL;
  return makeConnectionReleaseEvent(closedLink);
}

IOEvent*
ConnectMgr::processReadReadyEvent(int* listen_status)
{
  if ( *listen_status & corex::io::IOObjectsPool::OK_READ_READY ) {
    smsc_log_debug(_logger, "ConnectMgr::processReadReadyEvent::: try process all ready input streams");
    corex::io::InputStream* iStream;
    while ( (iStream = _ioObjectsPool.getNextReadyInputStream()) ) {
      corex::io::IOObject* curSocket = iStream->getOwner();
      std::string curSocketFingerPrint = curSocket->toString();
      smsc_log_debug(_logger, "ConnectMgr::processReadReadyEvent::: got InputStream for socket [=%s]", curSocketFingerPrint.c_str());
      socket2link_t::iterator s2lnk_map_iter;

      {
        smsc::core::synchronization::MutexGuard guard(_sock2link_lock);
        s2lnk_map_iter=_socket2link.find(curSocketFingerPrint);
        if ( s2lnk_map_iter != _socket2link.end() )
          _curConnect = s2lnk_map_iter->second;
        else
          _curConnect = NULL;
      }

      if ( _curConnect.Get() ) {
        smsc_log_debug(_logger, "ConnectMgr::processReadReadyEvent::: found connection for socket [%s], connectId=%s", curSocketFingerPrint.c_str(), _curConnect->getLinkId().getValue().c_str());

        IOEvent* ioEvent = readPacketAndMakeEvent(_curConnect, true);
        if ( ioEvent ) return ioEvent;
      }
    }
    *listen_status &= ~corex::io::IOObjectsPool::OK_READ_READY;
  }
  return NULL;
}

IOEvent*
ConnectMgr::createNewConnectIndicationEvent(const std::string& server_socket_fingerprint)
{
  socket2connectAcceptor_t::iterator s2ca_map_iter;
  smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);
  if ( (s2ca_map_iter=_socket2connectAcceptor.find(server_socket_fingerprint)) != _socket2connectAcceptor.end()) {
    ConnectAcceptor* connectAcceptor = s2ca_map_iter->second;
    smsc_log_debug(_logger, "ConnectMgr::createNewConnectIndicationEvent::: found connectAcceptor object with name=[%s] for active server socket object=[%s]", connectAcceptor->getName().c_str(), server_socket_fingerprint.c_str());
    Link* newConnection = connectAcceptor->accept();

    smsc_log_info(_logger, "ConnectMgr::createNewConnectIndicationEvent::: new connection accepted (connectionId=[%s]/socket=[%s]", newConnection->getLinkId().getValue().c_str(), newConnection->getSocket()->toString().c_str());

    addLink(newConnection->getLinkId(), newConnection);

    accptrs_to_connects_t::iterator accptr2connIter = _acceptor2establishedConnects.find(connectAcceptor->getName());
    if ( accptr2connIter == _acceptor2establishedConnects.end() ) {
      std::set<LinkId>* linkIdSet = new std::set<LinkId>();
      linkIdSet->insert(newConnection->getLinkId());
      _acceptor2establishedConnects.insert(std::make_pair(connectAcceptor->getName(), linkIdSet));
    } else
      accptr2connIter->second->insert(newConnection->getLinkId());

    return connectAcceptor->createIOEvent(newConnection->getLinkId()); // create event on preallocated memory
  } else {
    smsc_log_error(_logger, "ConnectMgr::createNewConnectIndicationEvent::: can't find connect accceptor for serverSocket [=%s]", server_socket_fingerprint.c_str());
    throw smsc::util::Exception("ConnectMgr::createNewConnectIndicationEvent::: can't find connect acceptor for active server socket");
  }
}

IOEvent*
ConnectMgr::processAcceptReadyEvent(int* listen_status)
{
  if ( *listen_status & corex::io::IOObjectsPool::OK_ACCEPT_READY ) {
    corex::io::network::ServerSocket* serverSocket;
    smsc_log_debug(_logger, "ConnectMgr::processAcceptReadyEvent::: try process next ready server socket");
    if ( (serverSocket = _ioObjectsPool.getNextReadyServerSocket()) ) {
      std::string curServerSocketFingerPrint = serverSocket->toString();
      smsc_log_debug(_logger, "ConnectMgr::processAcceptReadyEvent::: next serverSocket=[%s] is ready", curServerSocketFingerPrint.c_str());

      return createNewConnectIndicationEvent(curServerSocketFingerPrint);
    }
    smsc_log_debug(_logger, "ConnectMgr::processAcceptReadyEvent::: no ready server socket found");
    *listen_status &= ~corex::io::IOObjectsPool::OK_ACCEPT_READY;
  }
  return NULL;
}

IOEvent*
ConnectMgr::getEvent()
{
  smsc::core::synchronization::MutexGuard guard(_synchronize);
  if ( _shutdownInProgress ) throw smsc::util::Exception("ConnectMgr::getEvent::: shutdown in progress");

  smsc_log_debug(_logger, "ConnectMgr::getEvent::: Enter it, _curConnectId=%s", (_curConnect.Get() ? _curConnect->getLinkId().getValue().c_str() : "NULL"));

  IOEvent* ioEvent = NULL;
  try {
    if ( _curConnect.Get() )
      ioEvent = readPacketAndMakeEvent(_curConnect);

    if ( ioEvent ) return ioEvent;
    else _curConnect = NULL;

    while (true) {
      smsc_log_debug(_logger, "ConnectMgr::getEvent::: _listenStatus=%d", _listenStatus);

      if ( (ioEvent = processReadReadyEvent(&_listenStatus)) ||
          (ioEvent = processAcceptReadyEvent(&_listenStatus)) )
        return ioEvent;

      smsc_log_debug(_logger, "ConnectMgr::getEvent::: call _ioObjectsPool.listen()");
      _listenStatus = _ioObjectsPool.listen(); //waiting for events on sockets

      smsc_log_debug(_logger, "ConnectMgr::getEvent::: _ioObjectsPool.listen returned, _listenStatus=%d", _listenStatus);
    }
  } catch(corex::io::EOFException& ex) {
    smsc_log_info(_logger, "ConnectMgr::processReadReadyEvent::: connection for socket with id=[%s] closed by remote side", _curConnect->getLinkId().getValue().c_str());
    return cleanupLinkInfo(_curConnect->getLinkId());
  }
  return 0;//make compiler happy
}

bool
ConnectMgr::sendToLinkSet(const LinkId& out_link_id, const Message& message,
                          LinkId* used_link_id_in_linkset)
{
  LinkSetPtr foundLinkSet;

  {
    smsc::core::synchronization::MutexGuard guard(_linkSetLock);
    linksets_t::iterator linksets_iter = _linkSets.find(out_link_id);
    if ( linksets_iter != _linkSets.end() )
      foundLinkSet = linksets_iter->second;
  }

  if ( foundLinkSet.Get() ) {
    smsc_log_debug(_logger, "ConnectMgr::sendToLinkSet::: found LinkSet object for linkSetId value=[%s]", out_link_id.getValue().c_str());
    foundLinkSet->send(message);
    *used_link_id_in_linkset = foundLinkSet->getLinkId();
    return true;
  } else {
    smsc_log_debug(_logger, "ConnectMgr::sendToLinkSet::: LinkSet object not found for linkSetId value=[%s]", out_link_id.getValue().c_str());
    return false;
  }
}

void
ConnectMgr::sendToLink(const LinkId& out_link_id, const Message& message)
{
  LinkPtr foundConnection;

  {
    smsc::core::synchronization::MutexGuard guard(_linkLock);
    links_t::iterator activeLink_iter = _activeLinks.find(out_link_id);
    if ( activeLink_iter != _activeLinks.end() )
      foundConnection = activeLink_iter->second;
  }

  if ( foundConnection.Get() ) {
    smsc_log_debug(_logger, "ConnectMgr::sendToLink::: found Link object for linkId value=[%s]", out_link_id.getValue().c_str());
    foundConnection->send(message);
  } else {
    smsc_log_error(_logger, "ConnectMgr::sendToLink::: Link object not found for linkId value=[%s]", out_link_id.getValue().c_str());
    throw RouteNotFound(UNQUALIFIED, "ConnectMgr::sendToLink");
  }
}

LinkId
ConnectMgr::send(const LinkId& out_link_id, const Message& message)
{
  smsc_log_info(_logger, "ConnectMgr::send::: output message=[%s] to link [linkId=%s]", message.toString().c_str(), out_link_id.getValue().c_str());
  LinkId usedLinkIdInLinkSet;
  if ( sendToLinkSet(out_link_id, message, &usedLinkIdInLinkSet) ) return usedLinkIdInLinkSet;

  sendToLink(out_link_id, message);
  return out_link_id;
}

void
ConnectMgr::sendToLinkSetBroadcast(const LinkId& out_linkset_id,
                                   const Message& message)
{
  LinkSetPtr foundLinkSet;

  {
    smsc::core::synchronization::MutexGuard guard(_linkSetLock);
    linksets_t::iterator linksets_iter = _linkSets.find(out_linkset_id);
    if ( linksets_iter != _linkSets.end() )
      foundLinkSet = linksets_iter->second;
  }

  if ( foundLinkSet.Get() ) {
    smsc_log_info(_logger, "ConnectMgr::sendBroadcast::: found LinkSet object for linkSetId value=[%s]", out_linkset_id.getValue().c_str());
    foundLinkSet->sendBroadcast(message);
  } else
    smsc_log_info(_logger, "ConnectMgr::sendBroadcast::: LinkSet object not found for linkSetId value=[%s]", out_linkset_id.getValue().c_str());
}

void
ConnectMgr::changeProtocolState(const LinkId& out_link_id, const Message& message)
{
  smsc::core::synchronization::MutexGuard guard(_linkLock);
  links_t::iterator iter = _activeLinks.find(out_link_id);
  if ( iter != _activeLinks.end() )
    iter->second->changeProtocolState(message);
}

void
ConnectMgr::registerLinkSet(const LinkId& linkset_id, LinkSet::linkset_mode_t linkset_mode)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  if ( _linkSets.find(linkset_id) == _linkSets.end() ) {
    smsc_log_info(_logger, "ConnectMgr::registerLinkSet::: add new linkSet (linkSetId=[%s]/linkSetMode=[%s])", linkset_id.getValue().c_str(), LinkSet::getLinksetModeStringPresentation(linkset_mode));
    _linkSets.insert(std::make_pair(linkset_id, LinkSetPtr(new LinkSet(linkset_mode, linkset_id))));
  }
}

void
ConnectMgr::addLinkToLinkSet(const LinkId& linkset_id, const LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  linksets_t::iterator iter = _linkSets.find(linkset_id);
  if ( iter != _linkSets.end() ) {
    smsc::core::synchronization::MutexGuard linkLockGuard(_linkLock);
    links_t::iterator activeLinkIter = _activeLinks.find(link_id);
    if ( activeLinkIter != _activeLinks.end() ) {
      iter->second->addConnection(activeLinkIter->second.Get());
      link_to_linksets_t::iterator l2linkset_iter = _linkToLinkSets.find(link_id);
      if ( l2linkset_iter != _linkToLinkSets.end() )
        l2linkset_iter->second->insert(linkset_id);
      else {
        std::set<LinkId>* associatedLinkSets = new std::set<LinkId>();
        associatedLinkSets->insert(linkset_id);
        _linkToLinkSets.insert(std::make_pair(link_id, associatedLinkSets));
      }
      smsc_log_info(_logger, "ConnectMgr::addLinkToLinkSet::: link has been added into linkSet (linkId=[%s],linkSetId=[%s])", link_id.getValue().c_str(), linkset_id.getValue().c_str());
    }
  }
}

LinkPtr
ConnectMgr::removeLinkFromLinkSet(const LinkId& linkset_id, const LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  smsc_log_debug(_logger, "ConnectMgr::removeLinkFromLinkSet::: try remove link (linkId=[%s]) from linkSet (linkSetId=[%s])", link_id.getValue().c_str(), linkset_id.getValue().c_str());
  linksets_t::iterator iter = _linkSets.find(linkset_id);
  if ( iter != _linkSets.end() ) {
    smsc::core::synchronization::MutexGuard linkLockGuard(_linkLock);
    links_t::iterator activeLinkIter = _activeLinks.find(link_id);
    if ( activeLinkIter != _activeLinks.end() ) {
      iter->second->removeConnection(activeLinkIter->second.Get());

      link_to_linksets_t::iterator l2linkset_iter = _linkToLinkSets.find(link_id);
      if (l2linkset_iter != _linkToLinkSets.end()) {
        l2linkset_iter->second->erase(linkset_id);
        smsc_log_info(_logger, "ConnectMgr::removeLinkFromLinkSet::: link has been removed from linkSet (linkId=[%s],linkSetId=[%s])", link_id.getValue().c_str(), linkset_id.getValue().c_str());

        if ( l2linkset_iter->second->empty() ) {
          delete l2linkset_iter->second;
          _linkToLinkSets.erase(l2linkset_iter);
        }
      }
      return activeLinkIter->second;
    }
  }
  return LinkPtr(NULL);
}

void
ConnectMgr::removeLinkFromLinkSets(const std::set<LinkId>& linkset_ids, const LinkId& link_id)
{
  for(std::set<LinkId>::const_iterator iter = linkset_ids.begin(), end_iter = linkset_ids.end();
      iter != end_iter; ++iter)
    removeLinkFromLinkSet(*iter, link_id);
}

std::set<LinkId>
ConnectMgr::getLinkSetIds(const LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  smsc::core::synchronization::MutexGuard linkLockGuard(_linkLock);
  if ( _activeLinks.find(link_id) == _activeLinks.end() )
    return std::set<LinkId>();

  link_to_linksets_t::iterator iter = _linkToLinkSets.find(link_id);
  if ( iter != _linkToLinkSets.end() )
    return *(iter->second);
  else
    return std::set<LinkId>();
}

void
ConnectMgr::notifyShutdownInProgess()
{
  _shutdownInProgress = true;
}

}}}}
