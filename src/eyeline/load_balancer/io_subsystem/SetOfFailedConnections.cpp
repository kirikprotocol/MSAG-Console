#include <memory>

#include "eyeline/corex/io/IOExceptions.hpp"
#include "SetOfFailedConnections.hpp"
#include "IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

SetOfFailedConnections::SetOfFailedConnections(IOProcessor& io_processor,
                                               const LinkId& link_set_id)
  : _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _ioProcessor(io_processor), _relatedLinkSetId(link_set_id)
{}

SetOfFailedConnections::~SetOfFailedConnections()
{
  try {
    cleanup();
  } catch (...) {}
}

void
SetOfFailedConnections::addLink(const LinkRefPtr& link)
{
  _failedConnections.push_back(link);
}

SetOfNotBindedConnections*
SetOfFailedConnections::reestablishConnections()
{
  smsc_log_debug(_logger, "SetOfFailedConnections::reestablishConnections::: check if related linkset is empty");
  if ( getRelatedLinkSet()->isEmpty() ) {
    cleanup();
    return NULL;
  }

  std::auto_ptr<SetOfNotBindedConnections> notBindedLinks;
  for (failed_conns_t::iterator iter = _failedConnections.begin(), end_iter = _failedConnections.end();
       iter != end_iter;) {
    try {
      smsc_log_debug(_logger, "SetOfFailedConnections::reestablishConnections::: try establish connection to [%s]",
                     (*iter)->getPeerInfoString().c_str());
      (*iter)->establish();
      LinkRefPtr& linkPtr = *iter;
      smsc_log_info(_logger, "SetOfFailedConnections::reestablishConnections::: connection to [%s] established",
                     linkPtr->getPeerInfoString().c_str());
      if ( !notBindedLinks.get() )
        notBindedLinks.reset(new SetOfNotBindedConnections(_ioProcessor, _relatedLinkSetId));
      notBindedLinks->addLink(linkPtr->getLinkId());

      LinkSetRefPtr linkSet = _ioProcessor.getLinkSet(_relatedLinkSetId);
      if ( !linkSet.Get() )
        throw smsc::util::Exception("SetOfFailedConnections::reestablishConnections::: there isn't linkset with id='%s'",
                                    _relatedLinkSetId.toString().c_str());
      _ioProcessor.addLink(linkPtr);
      linkSet->addLink(linkPtr);
      _failedConnections.erase(iter++);
    } catch (corex::io::ConnectionFailedException& ex) {
      smsc_log_debug(_logger, "SetOfFailedConnections::reestablishConnections::: connection to [%s] failed, (reason: %s)",
                     (*iter)->getPeerInfoString().c_str(), ex.what());
      ++iter;
    } catch (...) {
      _failedConnections.erase(iter);
      throw;
    }
  }

  return notBindedLinks.release();
}

bool
SetOfFailedConnections::isEmpty() const
{
  return _failedConnections.empty();
}

const LinkId&
SetOfFailedConnections::getLinkSetId() const
{
  return getRelatedLinkSet()->getLinkId();
}

void
SetOfFailedConnections::cleanup()
{
  while (!_failedConnections.empty())
    _failedConnections.pop_front();
}

LinkSetRefPtr
SetOfFailedConnections::getRelatedLinkSet() const
{
  LinkSetRefPtr relatedLinkSet = _ioProcessor.getLinkSet(_relatedLinkSetId);
  if ( relatedLinkSet.Get() )
    return relatedLinkSet;
  else
    throw smsc::util::Exception("SetOfFailedConnections::getRelatedLinkSet::: linkSet with id='%s' is not registered",
                                _relatedLinkSetId.toString().c_str());
}

}}}
