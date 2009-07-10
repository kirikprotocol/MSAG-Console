#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_BINDER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_BINDER_HPP__

# include <map>

# include "logger/Logger.h"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/SetOfNotBindedConnections.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class Binder {
public:
  explicit Binder(IOProcessor& io_processor);
  LinkId addSetOfNotBindedConnections(SetOfNotBindedConnections* established_and_not_binded_connections);

  template <class BIND_REQUEST>
  void bind(const LinkId& link_set_id_to_smsc, const BIND_REQUEST& bindRequest);

  virtual void rebind(const LinkId& link_set_id_to_smsc) = 0;

  bool commitBindResponse(const LinkId& link_id_to_smsc);
  bool processFailedBindResponse(const LinkId& link_id_to_smsc);

  void removeBindingInfo(const LinkId& link_set_id_to_smsc);

protected:
  typedef std::map<LinkId, SetOfNotBindedConnections*> linksetid_to_notbinded_conns_map_t;

  struct NotbindedLinksSearchInfo {
    NotbindedLinksSearchInfo()
    : notBindedLinks(NULL) {}
    NotbindedLinksSearchInfo(SetOfNotBindedConnections* not_binded_links,
                             linksetid_to_notbinded_conns_map_t::iterator it)
    : notBindedLinks(not_binded_links), iter(it)
    {}
    SetOfNotBindedConnections* notBindedLinks;
    linksetid_to_notbinded_conns_map_t::iterator iter;
  };
  NotbindedLinksSearchInfo
  getSetOfNotBindedConnectionsContainingThisLink(const LinkId& linkIdToSmsc);

private:
  IOProcessor& _ioProcessor;
  smsc::logger::Logger* _logger;

  linksetid_to_notbinded_conns_map_t _knownSetsOfNotBindedConns;
  smsc::core::synchronization::Mutex _knownSetsOfNotBindedConnsLock;

  Binder(const Binder& rhs);
  Binder& operator=(const Binder& rhs);
};

# include "eyeline/load_balancer/io_subsystem/Binder_impl.hpp"

}}}

#endif
