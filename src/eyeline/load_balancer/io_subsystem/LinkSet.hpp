#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_LINKSET_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_LINKSET_HPP__

# include <sys/types.h>
# include <string>
# include <map>
# include <vector>

# include "logger/Logger.h"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/io_subsystem/Message.hpp"
# include "eyeline/load_balancer/io_subsystem/Connection.hpp"
# include "core/synchronization/RWLock.hpp"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/types.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class Link;

class LinkSet : public Connection {
public:
  LinkSet(const LinkId& link_id, unsigned total_number_of_links);
  virtual ~LinkSet() {
    delete [] _arrayOfLinks;
  }
  void addLink(LinkRefPtr& link);
  bool removeLink(const LinkId& link_id);

  class LinkSetIterator;
  LinkSetIterator getIterator();

  virtual Packet* receive() {
    throw utilx::UnsupportedOperationException("LinkSet::receive::: operation isn't supported for linkset");
  }

  using Connection::send;
  LinkId send(const Message& message, unsigned link_num_in_set);

  virtual corex::io::IOObject& getIOObject() {
    throw utilx::UnsupportedOperationException("LinkSet::getIOObject::: operation isn't supported for linkset");
  }
  bool isEmpty() const;

  virtual unsigned getMaxNumberOfLinks() const = 0;

  std::vector<bool> getActivityIndicators() const;
protected:
  smsc::logger::Logger* _logger;
  unsigned _totalNumberOfLinks;
  LinkId *_arrayOfLinks;
  unsigned _actualNumOfLinks;
  typedef std::map<LinkId, LinkRefPtr> links_t;
  links_t _links;
  std::vector<bool> _activeNodes;
  mutable smsc::core::synchronization::RWLock _lock;
  static const unsigned MAX_SMSMC_IN_LINKSET = 16;
public:
  class LinkSetIterator {
  public:
    ~LinkSetIterator();
    bool hasElement() const;
    void next();

    const Link& getCurrentElement() const;

    Link& getCurrentElement();

    void deleteCurrentElement();
  private:
    LinkSetIterator(links_t& links, smsc::core::synchronization::RWLock& lock);
    links_t::iterator _iter, _endIter;
    links_t& _container;
    smsc::core::synchronization::RWLock& _lock;
    friend class LinkSet;
  };

  static const LinkId EMPTY_LINK_ID;
};

class LinkSetSendException  : public smsc::util::Exception {
public:
  LinkSetSendException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}

#endif
