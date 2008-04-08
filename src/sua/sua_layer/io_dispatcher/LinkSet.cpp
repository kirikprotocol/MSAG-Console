#include <sua/sua_layer/io_dispatcher/LinkSet.hpp>

namespace io_dispatcher {

LinkSet::LinkSet(linkset_mode_t mode, const communication::LinkId& linkSetId)
  : _mode(mode), _wasConnIterInitilized(false), _logger(smsc::logger::Logger::getInstance("io_dsptch"))
{
  setLinkId(linkSetId);
}

void
LinkSet::addConnection(Link* connect)
{
  _connections.push_back(connect);
  connect->setWhole(this);
}

void
LinkSet::removeConnection(Link* connect)
{
  for(connections_lst_t::iterator iter=_connections.begin(), end_iter=_connections.end();
      iter != end_iter; ++iter) {
    if ( *iter == connect ) {
      _connections.erase(iter); break;
    }
  }
  connect->setWhole(NULL);
}


void
LinkSet::send(const communication::Message& message)
{
  if ( _connections.empty() ) {
    smsc_log_error(_logger, "LinkSet::send::: there aren't connections in LinkSet");
    return;
  }

  if ( !_wasConnIterInitilized ) {
    _lastUsedConnIter = _connections.begin();
    _wasConnIterInitilized = true;
  }

  if ( _mode == OVERRIDE) {
    // send message to first connection in list
    (*_lastUsedConnIter)->send(message);
  } else if ( _mode = LOADSHARE ) {
    // send message in round-robin way
    (*_lastUsedConnIter)->send(message);
    ++_lastUsedConnIter;
    if (_lastUsedConnIter == _connections.end())
      _lastUsedConnIter = _connections.begin();
  } else if ( _mode = BROADCAST ) {
    connections_lst_t::iterator end_iter = _connections.end();
    while(_lastUsedConnIter != end_iter) {
      (*_lastUsedConnIter)->send(message);
    }
    _lastUsedConnIter = _connections.begin();
    // send message to all connections in set
  } else {
    smsc_log_error(_logger, "LinkSet::send::: unknown linset mode value [=%d]", _mode);
    throw smsc::util::Exception("LinkSet::send::: unknown linset mode value [=%d]", _mode);
  }
}

communication::TP*
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
  return ConnectionIterator(_connections);
}

LinkSet::ConnectionIterator::ConnectionIterator(LinkSet::connections_lst_t& container)
  : _container(container), _iter(container.begin()), _endIter(container.end()) {}

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

LinkSet::ConnectionIterator::const_ref
LinkSet::ConnectionIterator::getCurrentElement() const
{
  return *_iter;
}

LinkSet::ConnectionIterator::ref
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

}
