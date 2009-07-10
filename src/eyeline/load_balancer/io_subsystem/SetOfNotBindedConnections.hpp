#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_SETOFNOTBINDEDCONNECTIONS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_SETOFNOTBINDEDCONNECTIONS_HPP__

# include <set>
# include "logger/Logger.h"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/types.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkSet.hpp"
# include "eyeline/load_balancer/io_subsystem/Message.hpp"
# include "eyeline/load_balancer/io_subsystem/TimeoutEvent.hpp"
# include "eyeline/load_balancer/io_subsystem/TimeoutMonitor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class SetOfNotBindedConnections {
public:
  SetOfNotBindedConnections(IOProcessor& io_processor,
                            const LinkId& link_set_id);

  void addLink(LinkId linkId);
  void addLinks(SetOfNotBindedConnections* another_not_binded_conns);

  LinkId getLinkSetId() const;

  template <class BIND_REQUEST>
  void bindConnections(const BIND_REQUEST& bind_request);

  bool commitBindedConnection(const LinkId& link_id_to_smsc);
  bool processFailedBindResponse(const LinkId& link_id_to_smsc);
  bool isEmpty() const;
  bool isLinkBelongToLinkSet(const LinkId& link_id_to_smsc);

protected:
  LinkSetRefPtr getRelatedLinkSet() const;

private:
  smsc::logger::Logger* _logger;
  IOProcessor& _ioProcessor;
  LinkId _relatedLinkSetId;
  bool _positiveBindResponseHasNotBeenGotYet;
  typedef std::set<LinkId> not_binded_conns_t;
  not_binded_conns_t _notBindedConnections;
  unsigned int _totalLinks, _numOfBindFailedLinks;
};

# include "eyeline/load_balancer/io_subsystem/SetOfNotBindedConnections_impl.hpp"

}}}

#endif
