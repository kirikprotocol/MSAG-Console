#include "core/synchronization/RWLockGuard.hpp"
#include "Link.hpp"
#include "LinkSet.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

const LinkId LinkSet::EMPTY_LINK_ID;

LinkSet::LinkSet(const LinkId& link_id, unsigned total_number_of_links)
  : _logger(smsc::logger::Logger::getInstance("io_subsys")),
    _totalNumberOfLinks(total_number_of_links),
    _arrayOfLinks(new LinkId[total_number_of_links]), _actualNumOfLinks(0)
{
  setLinkId(link_id);
}

void
LinkSet::addLink(Link* link)
{
  smsc::core::synchronization::WriteLockGuard synchronize(_lock);
  _links.insert(std::make_pair(link->getLinkId(), link));
  _arrayOfLinks[_actualNumOfLinks++] = link->getLinkId();
  LinkId emptyLinkId;
  if ( link->getRelatedLinkSetId() == emptyLinkId )
    link->setLinkSetId(getLinkId());
  else if ( link->getRelatedLinkSetId() != getLinkId() )
    throw smsc::util::Exception("LinkSet::addLink::: link being added [id='%s'] pertain to another linkset with id='%s'",
                                link->getLinkId().toString().c_str(), link->getRelatedLinkSetId().toString().c_str());
}

bool
LinkSet::removeLink(const LinkId& link_id)
{
  smsc::core::synchronization::WriteLockGuard synchronize(_lock);
  links_t::iterator iter = _links.find(link_id);
  if ( iter == _links.end() )
    return _actualNumOfLinks == 0;

  iter->second->resetLinkSetId();
  _links.erase(iter);
  for(unsigned i=0; i<_actualNumOfLinks; ++i) {
    if ( _arrayOfLinks[i] == link_id ) {
      _arrayOfLinks[i] = EMPTY_LINK_ID;

      if ( i == _actualNumOfLinks - 1 ) --_actualNumOfLinks;
      break;
    }
  }

  return _actualNumOfLinks == 0;
}

LinkSet::LinkSetIterator
LinkSet::getIterator()
{
  return LinkSetIterator(_links, _lock);
}

LinkId
LinkSet::send(const Message& message, unsigned link_num_in_set)
{
  smsc::core::synchronization::ReadLockGuard synchronize(_lock);
  if ( link_num_in_set < _actualNumOfLinks ) {
    LinkId linkIdForUse = _arrayOfLinks[link_num_in_set];
    if ( linkIdForUse != EMPTY_LINK_ID ) {
      _links[linkIdForUse]->send(message);

      return linkIdForUse;
    } else
      return EMPTY_LINK_ID;
  }

  throw LinkSetSendException("LinkSet::send::: can't send message to specified link with num=%d - such link doest't exist",
                             link_num_in_set);
}

bool
LinkSet::isEmpty() const
{
  smsc::core::synchronization::ReadLockGuard synchronize(_lock);
  return _links.empty();
}

bool
LinkSet::LinkSetIterator::hasElement() const
{
  return _iter != _endIter;
}

void
LinkSet::LinkSetIterator::next()
{
  ++_iter;
}

const Link&
LinkSet::LinkSetIterator::getCurrentElement() const
{
  return *_iter->second;
}

Link&
LinkSet::LinkSetIterator::getCurrentElement()
{
  return *_iter->second;
}

void
LinkSet::LinkSetIterator::deleteCurrentElement()
{
  _container.erase(_iter++);
}

LinkSet::LinkSetIterator::LinkSetIterator(links_t& links, smsc::core::synchronization::RWLock& lock)
  : _lock(lock), _container(links)
{
  _lock.wlock();
  _iter = links.begin(); _endIter = links.end();
}

LinkSet::LinkSetIterator::~LinkSetIterator()
{
  try {
    _lock.unlock();
  } catch (...) {}
}

}}}
