#include <utility>
#include <assert.h>
#include <core/synchronization/MutexGuard.hpp>
#include <eyeline/corex/io/network/Socket.hpp>
#include <eyeline/utilx/PreallocatedMemoryManager.hpp>
#include <eyeline/sua/communication/TP.hpp>
#include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

#include "GotMessageEvent.hpp"
#include "ConnectionReleaseEvent.hpp"
#include "Connection.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

void
ConnectMgr::addConnectAcceptor(const std::string& acceptorName, ConnectAcceptor* connectAcceptor)
{
  {
    smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);
    smsc_log_info(_logger, "ConnectMgr::addConnectAcceptor::: add ConnectAcceptor: acceptorName=[%s], corresponding server socket=[%s]", acceptorName.c_str(), connectAcceptor->getServerSocket()->toString().c_str());

    _connectAcceptors.insert(std::make_pair(acceptorName, connectAcceptor));
    _socket2connectAcceptor.insert(std::make_pair(connectAcceptor->getServerSocket()->toString(), connectAcceptor));
  }

  _ioObjectsPool.insert(connectAcceptor->getServerSocket());
}

ConnectAcceptor*
ConnectMgr::removeConnectAcceptor(const std::string& acceptorName, bool closeAllEstablishedConnects)
{
  ConnectAcceptor* result=NULL;
  smsc_log_debug(_logger, "ConnectMgr::removeConnectAcceptor::: try remove ConnectAcceptor with acceptorName=[%s]", acceptorName.c_str());
  {
    smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);

    connect_acceptors_map_t::iterator iter = _connectAcceptors.find(acceptorName);
    if ( iter != _connectAcceptors.end() ) {
      result=iter->second;
      _socket2connectAcceptor.erase(result->getServerSocket()->toString());

      if ( closeAllEstablishedConnects ) {
        accptrs_to_connects_t::iterator accptr2connIter = _acceptor2establishedConnects.find(acceptorName);
        if ( accptr2connIter != _acceptor2establishedConnects.end() ) {
          std::set<communication::LinkId>* acceptedConnections = accptr2connIter->second;
          while ( !acceptedConnections->empty() ) {
            std::set<communication::LinkId>::iterator set_iter = acceptedConnections->begin();
            communication::LinkId linkId = *set_iter;
            acceptedConnections->erase(set_iter);
            removeLink(linkId, false);
          }
          delete acceptedConnections;
        }
      }
      _connectAcceptors.erase(iter);
      smsc_log_info(_logger, "ConnectMgr::removeConnectAcceptor::: ConnectAcceptor with acceptorName=[%s] has been removed; corresponding server socket=[%s]", acceptorName.c_str(), result->getServerSocket()->toString().c_str());
    }
  }

  if ( result )
    _ioObjectsPool.remove(result->getServerSocket());

  return result;
}

void
ConnectMgr::addLink(const communication::LinkId& linkId, Link* link)
{
  smsc_log_debug(_logger, "ConnectMgr::addLink::: try add link=[%s]", linkId.getValue().c_str());
  LinkPtr linkPtr(link);
  {
    smsc::core::synchronization::MutexGuard guard(_linkLock);
    _activeLinks.insert(std::make_pair(linkId, linkPtr));
  }

  _ioObjectsPool.insert(link->getSocket()->getInputStream());

  {
    smsc::core::synchronization::MutexGuard guard(_sock2link_lock);
    _socket2link.insert(std::make_pair(link->getSocket()->toString(), linkPtr));
  }

  smsc_log_info(_logger, "ConnectMgr::addLink::: link=[%s] has been added", linkPtr->getLinkId().getValue().c_str());
}

LinkPtr
ConnectMgr::removeLink(const communication::LinkId& linkId, bool cleanUpAcceptorEstablishedConns /* =true */)
{
  smsc_log_debug(_logger, "ConnectMgr::removeLink::: try remove link=[%s]", linkId.getValue().c_str());
  LinkPtr curConnect;
  {
    smsc::core::synchronization::MutexGuard guard(_linkLock);
    links_t::iterator iter = _activeLinks.find(linkId);
    if ( iter != _activeLinks.end() ) {
      curConnect = iter->second;
      _activeLinks.erase(iter);
    }
  }
  if ( curConnect.Get() != NULL ) {
    const ConnectAcceptor* connectCreator = curConnect->getCreator();
    if ( cleanUpAcceptorEstablishedConns && connectCreator ) {
      smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);
      accptrs_to_connects_t::iterator accptr2connIter = _acceptor2establishedConnects.find(connectCreator->getName());
      if ( accptr2connIter != _acceptor2establishedConnects.end() ) {
        std::set<communication::LinkId> *links = accptr2connIter->second;
        links->erase(linkId);
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

ConnectMgr::ConnectMgr()
  : _listenStatus(corex::io::IOObjectsPool::OK_NO_EVENTS), _logger(smsc::logger::Logger::getInstance("io_dsptch")), _shutdownInProgress(false)
{}

IOEvent*
ConnectMgr::readPacketAndMakeEvent(LinkPtr& curConnect, bool fillUpInputBuffer /*=false*/)
{
  if ( fillUpInputBuffer )
    curConnect->bufferInputTransportPackets(); // read data of max possible size into buffer. 

  if ( curConnect->hasReadyTransportPacket() ) // check if buffered data contains the whole transport packet
  {
    smsc_log_debug(_logger, "ConnectMgr::readPacketAndMakeEvent::: call receive() for connect=[%s]", curConnect->getLinkId().getValue().c_str());
    communication::TP* transportPacket = curConnect->receive(); // return pointer to TP from preallocated memory
    utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_EVENT>();
    smsc_log_info(_logger, "ConnectMgr::readPacketAndMakeEvent::: new packet has been read from connect=[%s], packetType=%d, packetLen=%d", curConnect->getLinkId().getValue().c_str(), transportPacket->packetType, transportPacket->packetLen);

    return new (ptr_desc->allocated_memory) GotMessageEvent(transportPacket, curConnect->getLinkId());
  }
  return NULL;
}

IOEvent*
ConnectMgr::cleanupLinkInfo(const communication::LinkId& linkId)
{
  LinkPtr curConnect = removeLink(linkId);
  if ( curConnect.Get() ) {
    utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_EVENT>();
    ConnectionReleaseEvent* connReleaseEvent = new (ptr_desc->allocated_memory) ConnectionReleaseEvent(linkId);
    _curConnect = NULL;
    return connReleaseEvent;
  } else return NULL;
}

IOEvent*
ConnectMgr::processReadReadyEvent(int* listenStatus)
{
  if ( *listenStatus & corex::io::IOObjectsPool::OK_READ_READY ) {
    smsc_log_debug(_logger, "ConnectMgr::processReadReadyEvent::: try process all ready input streams");
    corex::io::InputStream* iStream;
    while ( iStream = _ioObjectsPool.getNextReadyInputStream() ) {
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
        smsc_log_info(_logger, "ConnectMgr::processReadReadyEvent::: found connection for socket [%s], connectId=%s", curSocketFingerPrint.c_str(), _curConnect->getLinkId().getValue().c_str());

        try {
          IOEvent* ioEvent = readPacketAndMakeEvent(_curConnect, true);
          if ( ioEvent ) return ioEvent;
        } catch (corex::io::EOFException& ex) {
          smsc_log_info(_logger, "ConnectMgr::processReadReadyEvent::: connection for socket [%s] closed by remote side", curSocketFingerPrint.c_str());
          IOEvent* eofIOEvent = cleanupLinkInfo(_curConnect->getLinkId());
          if ( eofIOEvent ) return eofIOEvent;
        }
      }
    }
    *listenStatus &= ~corex::io::IOObjectsPool::OK_READ_READY;
  }
  return NULL;
}

IOEvent*
ConnectMgr::createNewConnectIndicationEvent(const std::string& serverSocketFingerPrint)
{
  socket2connectAcceptor_t::iterator s2ca_map_iter;
  smsc::core::synchronization::MutexGuard guard(_connAcceptorsLock);
  if ( (s2ca_map_iter=_socket2connectAcceptor.find(serverSocketFingerPrint)) != _socket2connectAcceptor.end()) {
    ConnectAcceptor* connectAcceptor = s2ca_map_iter->second;
    smsc_log_debug(_logger, "ConnectMgr::createNewConnectIndicationEvent::: found connectAcceptor object with name=[%s] for active server socket object=[%s]", connectAcceptor->getName().c_str(), serverSocketFingerPrint.c_str());
    Link* newConnection = connectAcceptor->accept();

    smsc_log_info(_logger, "ConnectMgr::createNewConnectIndicationEvent::: new connection accepted (connectionId=[%s]/socket=[%s]", newConnection->getLinkId().getValue().c_str(), newConnection->getSocket()->toString().c_str());

    addLink(newConnection->getLinkId(), newConnection);

    accptrs_to_connects_t::iterator accptr2connIter = _acceptor2establishedConnects.find(connectAcceptor->getName());
    if ( accptr2connIter == _acceptor2establishedConnects.end() ) {
      std::set<communication::LinkId>* linkIdSet = new std::set<communication::LinkId>();
      linkIdSet->insert(newConnection->getLinkId());
      _acceptor2establishedConnects.insert(std::make_pair(connectAcceptor->getName(), linkIdSet));
    } else
      accptr2connIter->second->insert(newConnection->getLinkId());

    return connectAcceptor->createIOEvent(newConnection->getLinkId()); // create event on preallocated memory
  } else {
    smsc_log_error(_logger, "ConnectMgr::createNewConnectIndicationEvent::: can't find connect accceptor for serverSocket [=%s]", serverSocketFingerPrint.c_str());
    throw smsc::util::Exception("ConnectMgr::createNewConnectIndicationEvent::: can't find connect acceptor for active server socket");
  }

}

IOEvent*
ConnectMgr::processAcceptReadyEvent(int* listenStatus)
{
  if ( *listenStatus & corex::io::IOObjectsPool::OK_ACCEPT_READY ) {
    corex::io::network::ServerSocket* serverSocket;
    smsc_log_debug(_logger, "ConnectMgr::processAcceptReadyEvent::: try process next ready server socket");
    if ( serverSocket = _ioObjectsPool.getNextReadyServerSocket() ) {
      std::string curServerSocketFingerPrint = serverSocket->toString();
      smsc_log_info(_logger, "ConnectMgr::processAcceptReadyEvent::: next serverSocket=[%s] is ready", curServerSocketFingerPrint.c_str());

      return createNewConnectIndicationEvent(curServerSocketFingerPrint);
    }
    smsc_log_debug(_logger, "ConnectMgr::processAcceptReadyEvent::: no ready server socket found");
    *listenStatus &= ~corex::io::IOObjectsPool::OK_ACCEPT_READY;
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
}

bool
ConnectMgr::sendToLinkSet(const communication::LinkId& outLinkId, const communication::Message& message, communication::LinkId* usedLinkIdInLinkSet)
{
  LinkSetPtr foundLinkSet;

  {
    smsc::core::synchronization::MutexGuard guard(_linkSetLock);
    linksets_t::iterator linksets_iter = _linkSets.find(outLinkId);
    if ( linksets_iter != _linkSets.end() )
      foundLinkSet = linksets_iter->second;
  }

  if ( foundLinkSet.Get() ) {
    smsc_log_info(_logger, "ConnectMgr::sendToLinkSet::: found LinkSet object for linkSetId value=[%s]", outLinkId.getValue().c_str());
    foundLinkSet->send(message);
    *usedLinkIdInLinkSet = foundLinkSet->getLinkId();
    return true;
  } else {
    smsc_log_info(_logger, "ConnectMgr::sendToLinkSet::: LinkSet object not found for linkSetId value=[%s]", outLinkId.getValue().c_str());
    return false;
  }
}

void
ConnectMgr::sendToLink(const communication::LinkId& outLinkId, const communication::Message& message)
{
  LinkPtr foundConnection;

  {
    smsc::core::synchronization::MutexGuard guard(_linkLock);
    links_t::iterator activeLink_iter = _activeLinks.find(outLinkId);
    if ( activeLink_iter != _activeLinks.end() )
      foundConnection = activeLink_iter->second;
  }

  if ( foundConnection.Get() ) {
    smsc_log_info(_logger, "ConnectMgr::sendToLink::: found Link object for linkId value=[%s]", outLinkId.getValue().c_str());
    foundConnection->send(message);
  } else
    smsc_log_info(_logger, "ConnectMgr::sendToLink::: Link object not found for linkId value=[%s]", outLinkId.getValue().c_str());
}

communication::LinkId
ConnectMgr::send(const communication::LinkId& outLinkId, const communication::Message& message)
{
  smsc_log_info(_logger, "ConnectMgr::send::: try send message=[%s] to link [linkId=%s]", message.toString().c_str(), outLinkId.getValue().c_str());
  communication::LinkId usedLinkIdInLinkSet;
  if ( sendToLinkSet(outLinkId, message, &usedLinkIdInLinkSet) ) return usedLinkIdInLinkSet;

  sendToLink(outLinkId, message);
  return outLinkId;
}

void
ConnectMgr::sendToLinkSetBroadcast(const communication::LinkId& outLinkSetId, const communication::Message& outputMessage)
{
  LinkSetPtr foundLinkSet;

  {
    smsc::core::synchronization::MutexGuard guard(_linkSetLock);
    linksets_t::iterator linksets_iter = _linkSets.find(outLinkSetId);
    if ( linksets_iter != _linkSets.end() )
      foundLinkSet = linksets_iter->second;
  }

  if ( foundLinkSet.Get() ) {
    smsc_log_info(_logger, "ConnectMgr::sendBroadcast::: found LinkSet object for linkSetId value=[%s]", outLinkSetId.getValue().c_str());
    foundLinkSet->sendBroadcast(outputMessage);
  } else
    smsc_log_info(_logger, "ConnectMgr::sendBroadcast::: LinkSet object not found for linkSetId value=[%s]", outLinkSetId.getValue().c_str());
}

void
ConnectMgr::changeProtocolState(const communication::LinkId& outLinkId, const communication::Message& message)
{
  smsc::core::synchronization::MutexGuard guard(_linkLock);
  links_t::iterator iter = _activeLinks.find(outLinkId);
  if ( iter != _activeLinks.end() )
    iter->second->changeProtocolState(message);
}

void
ConnectMgr::registerLinkSet(const communication::LinkId& linkSetId, LinkSet::linkset_mode_t linkSetMode)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  if ( _linkSets.find(linkSetId) == _linkSets.end() ) {
    smsc_log_info(_logger, "ConnectMgr::registerLinkSet::: add new linkSet (linkSetId=[%s]/linkSetMode=[%s])", linkSetId.getValue().c_str(), LinkSet::getLinksetModeStringPresentation(linkSetMode));
    _linkSets.insert(std::make_pair(linkSetId, LinkSetPtr(new LinkSet(linkSetMode, linkSetId))));
  }
}

void
ConnectMgr::addLinkToLinkSet(const communication::LinkId& linkSetId, const communication::LinkId& linkId)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  linksets_t::iterator iter = _linkSets.find(linkSetId);
  if ( iter != _linkSets.end() ) {
    smsc::core::synchronization::MutexGuard linkLockGuard(_linkLock);
    links_t::iterator activeLinkIter = _activeLinks.find(linkId);
    if ( activeLinkIter != _activeLinks.end() ) {
      iter->second->addConnection(activeLinkIter->second.Get());
      link_to_linksets_t::iterator l2linkset_iter = _linkToLinkSets.find(linkId);
      if ( l2linkset_iter != _linkToLinkSets.end() )
        l2linkset_iter->second->insert(linkSetId);
      else {
        std::set<communication::LinkId>* associatedLinkSets = new std::set<communication::LinkId>();
        associatedLinkSets->insert(linkSetId);
        _linkToLinkSets.insert(std::make_pair(linkId, associatedLinkSets));
      }
      smsc_log_info(_logger, "ConnectMgr::addLinkToLinkSet::: link has been added into linkSet (linkId=[%s],linkSetId=[%s])", linkId.getValue().c_str(), linkSetId.getValue().c_str());
    }
  }
}

LinkPtr
ConnectMgr::removeLinkFromLinkSet(const communication::LinkId& linkSetId, const communication::LinkId& linkId)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  smsc_log_debug(_logger, "ConnectMgr::removeLinkFromLinkSet::: try remove link (linkId=[%s]) from linkSet (linkSetId=[%s])", linkId.getValue().c_str(), linkSetId.getValue().c_str());
  linksets_t::iterator iter = _linkSets.find(linkSetId);
  if ( iter != _linkSets.end() ) {
    smsc::core::synchronization::MutexGuard linkLockGuard(_linkLock);
    links_t::iterator activeLinkIter = _activeLinks.find(linkId);
    if ( activeLinkIter != _activeLinks.end() ) {
      iter->second->removeConnection(activeLinkIter->second.Get());

      link_to_linksets_t::iterator l2linkset_iter = _linkToLinkSets.find(linkId);
      if (l2linkset_iter != _linkToLinkSets.end()) {
        l2linkset_iter->second->erase(linkSetId);
        smsc_log_info(_logger, "ConnectMgr::removeLinkFromLinkSet::: link has been removed from linkSet (linkId=[%s],linkSetId=[%s])", linkId.getValue().c_str(), linkSetId.getValue().c_str());

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
ConnectMgr::removeLinkFromLinkSets(const std::set<communication::LinkId>& linkSetIds, const communication::LinkId& linkId)
{
  for(std::set<communication::LinkId>::const_iterator iter = linkSetIds.begin(), end_iter = linkSetIds.end();
      iter != end_iter; ++iter)
    removeLinkFromLinkSet(*iter, linkId);
}

std::set<communication::LinkId>
ConnectMgr::getLinkSetIds(const communication::LinkId& linkId)
{
  smsc::core::synchronization::MutexGuard guard(_linkSetLock);
  {
    smsc::core::synchronization::MutexGuard linkLockGuard(_linkLock);
    if ( _activeLinks.find(linkId) == _activeLinks.end() )
      return std::set<communication::LinkId>();

    link_to_linksets_t::iterator iter = _linkToLinkSets.find(linkId);
    if ( iter != _linkToLinkSets.end() )
      return *(iter->second);
    else
      return std::set<communication::LinkId>();
  }
}

void
ConnectMgr::notifyShutdownInProgess()
{
  _shutdownInProgress = true;
}

}}}}
