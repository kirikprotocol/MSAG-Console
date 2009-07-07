#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_SWITCHCIRCUITCONTROLLERTMPL_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_SWITCHCIRCUITCONTROLLERTMPL_HPP__

# include "eyeline/load_balancer/io_subsystem/SwitchCircuitController.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

template <class CONNECTION, class LINKSET>
class SwitchCircuitControllerTmpl : public SwitchCircuitController {
public:
  SwitchCircuitControllerTmpl(const std::string& link_set_prefix,
                              smsc::util::config::ConfigView& out_links_cfg_section,
                              SwitchingTable& switching_table)
  : SwitchCircuitController(link_set_prefix, out_links_cfg_section, switching_table),
    _linkSetSeqId(0)
  {}

protected:
  virtual Link* createIncomingLink(corex::io::network::TCPSocket* socket) {
    return new CONNECTION(socket);
  }

  virtual LinkSet* createLinkSet() {
    char linkSetSeqIdStr[32];
    sprintf(linkSetSeqIdStr, "_%d", _linkSetSeqId++);
    std::string linSetId = getLinkSetPrefix() + linkSetSeqIdStr;
    return new LINKSET(LinkId(linSetId), getTotalNumberOfOutLinks());
  }

  virtual Link* createOutcomingLink(const std::string& peer_host,
                                    unsigned peer_port,
                                    unsigned connect_timeout,
                                    unsigned bind_resp_wait_timeout,
                                    unsigned unbind_resp_wait_timeout) {
    return new CONNECTION(peer_host, peer_port, connect_timeout,
                          bind_resp_wait_timeout, unbind_resp_wait_timeout);
  }
private:
  unsigned _linkSetSeqId;
};

}}}

#endif

