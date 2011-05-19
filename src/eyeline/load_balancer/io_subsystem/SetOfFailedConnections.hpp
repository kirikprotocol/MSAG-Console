#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_SETOFFAILEDCONNECTIONS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_SETOFFAILEDCONNECTIONS_HPP__

# include <list>

# include "logger/Logger.h"
# include "eyeline/load_balancer/io_subsystem/types.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkSet.hpp"
# include "eyeline/load_balancer/io_subsystem/SetOfNotBindedConnections.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class SetOfFailedConnections {
public:
  SetOfFailedConnections(IOProcessor& io_processor,
                         const LinkId& link_set_id);
  ~SetOfFailedConnections();

  void addLink(const LinkRefPtr& link);
  SetOfNotBindedConnections* reestablishConnections();
  bool isEmpty() const;
  const LinkId& getLinkSetId() const;

protected:
  void cleanup();
  LinkSetRefPtr getRelatedLinkSet() const;

private:
  smsc::logger::Logger* _logger;
  IOProcessor& _ioProcessor;
  LinkId _relatedLinkSetId;
  typedef std::list<LinkRefPtr> failed_conns_t;
  failed_conns_t _failedConnections;
};

}}}

#endif
