#include "core/synchronization/RWLockGuard.hpp"
#include "eyeline/ss7na/common/io_dispatcher/LinkSet.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

LinkSet::LinkSet(linkset_mode_t mode, const LinkId& linkSetId)
  : _mode(mode), _lastUsedConnIter(_connections.begin()), _logger(smsc::logger::Logger::getInstance("io_dsptch"))
{
  setLinkId(linkSetId);
}

void
LinkSet::addConnection(Link* connect)
{
  smsc::core::synchronization::WriteLockGuard lock(_lock);
  _connections.push_back(connect);
  _lastUsedConnIter = _connections.begin();
  connect->setWhole(this);
}

void
LinkSet::removeConnection(Link* connect)
{
  smsc::core::synchronization::WriteLockGuard lock(_lock);
  for(connections_lst_t::iterator iter=_connections.begin(), end_iter=_connections.end();
      iter != end_iter; ++iter) {
    if ( *iter == connect ) {
      _connections.erase(iter);
      _lastUsedConnIter = _connections.begin();
      break;
    }
  }
  connect->setWhole(NULL);
}

void
LinkSet::send(const Message& message)
{
  smsc::core::synchronization::ReadLockGuard lock(_lock);

  if ( _connections.empty() ) {
    smsc_log_error(_logger, "LinkSet::send::: there aren't connections in LinkSet");
    throw smsc::util::Exception("LinkSet::send::: there aren't connections in LinkSet");
  }

  if ( _mode == OVERRIDE) {
    // send message to first connection in list
    Connection* curConnection = *(_connections.begin());
    curConnection->send(message);
  } else if ( _mode = LOADSHARE ) {
    // send message in round-robin way
    Connection* curConnection;

    _currentConnLock.Lock();
    curConnection = *_lastUsedConnIter++;
    if (_lastUsedConnIter == _connections.end())
      _lastUsedConnIter = _connections.begin();
    _currentConnLock.Unlock();

    curConnection->send(message);
  } else if ( _mode = BROADCAST ) {
    // send message to all connections in set
    for(connections_lst_t::iterator iter = _connections.begin(), end_iter = _connections.end();
        iter != end_iter; ++iter)
      (*iter)->send(message);
  } else {
    smsc_log_error(_logger, "LinkSet::send::: unknown linset mode value [=%d]", _mode);
    throw smsc::util::Exception("LinkSet::send::: unknown linset mode value [=%d]", _mode);
  }
}

void
LinkSet::sendBroadcast(const Message& message)
{
  smsc::core::synchronization::ReadLockGuard lock(_lock);

  if ( _connections.empty() ) {
    smsc_log_error(_logger, "LinkSet::send::: there aren't connections in LinkSet");
    return;
  }

  for(connections_lst_t::iterator iter = _connections.begin(), end_iter = _connections.end();
      iter != end_iter; ++iter)
    (*iter)->send(message);
}

TP*
LinkSet::receive()
{
  throw smsc::util::Exception("LinkSet::receive::: not supported operation");
}

bool
LinkSet::hasReadyTransportPacket()
{
  throw smsc::util::Exception("LinkSet::hasReadyTransportPacket::: not supported operation");
}

void
LinkSet::bufferInputTransportPackets()
{
  throw smsc::util::Exception("LinkSet::bufferInputTransportPackets::: not supported operation");
}

LinkSet::ConnectionIterator
LinkSet::getIterator()
{
  return ConnectionIterator(_connections, _lock);
}

LinkSet::ConnectionIterator::ConnectionIterator(LinkSet::connections_lst_t& container, smsc::core::synchronization::RWLock& lock)
  : _container(container), _iter(container.begin()), _endIter(container.end()), _lock(lock)
{
  _lock.wlock();
}

LinkSet::ConnectionIterator::~ConnectionIterator()
{
  _lock.unlock();
}

bool
LinkSet::ConnectionIterator::hasElement() const
{
  return _iter != _endIter;
}

void
LinkSet::ConnectionIterator::next()
{
  ++_iter;
}

const Connection*
LinkSet::ConnectionIterator::getCurrentElement() const
{
  return *_iter;
}

Connection*
LinkSet::ConnectionIterator::getCurrentElement()
{
  return *_iter;
}

void
LinkSet::ConnectionIterator::deleteCurrentElement()
{
  _iter = _container.erase(_iter);
}

const char*
LinkSet::getLinksetModeStringPresentation(linkset_mode_t mode)
{
  if ( mode == OVERRIDE )
    return "OVERRIDE";
  else if ( mode == LOADSHARE )
    return "LOADSHARE";
  else if ( mode == BROADCAST )
    return "BROADCAST";
  else
    return "UNKNOWN";
}

}}}}
