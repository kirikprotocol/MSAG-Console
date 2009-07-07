#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_SWITCHCIRCUITCONTROLLER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_SWITCHCIRCUITCONTROLLER_HPP__

# include <set>
# include <vector>
# include <string>
# include <memory>

# include "logger/Logger.h"
# include "util/Exception.hpp"
# include "util/config/ConfigView.h"

# include "eyeline/corex/io/IOExceptions.hpp"
# include "eyeline/corex/io/network/TCPSocket.hpp"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkSet.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class SwitchCircuitController {
public:
  SwitchCircuitController(const std::string& link_set_prefix,
                          smsc::util::config::ConfigView& out_links_cfg_section,
                          SwitchingTable& switching_table);

  void activateConnection(corex::io::network::TCPSocket* new_socket,
                          IOProcessor& io_processor);
  void deactivateConnection(const LinkId& link_id, IOProcessor& io_processor);

protected:
  virtual Link* createIncomingLink(corex::io::network::TCPSocket* socket) = 0;

  virtual LinkSet* createLinkSet() = 0;

  virtual Link* createOutcomingLink(const std::string& peer_host,
                                    unsigned int  peer_port,
                                    unsigned int connect_timeout,
                                    unsigned int bind_resp_wait_timeout,
                                    unsigned int unbind_resp_wait_timeout) = 0;

  void addOutConnectDefinition(const std::string& linkName,
                               const char* peer_host,
                               unsigned int peer_port,
                               unsigned int connect_timeout,
                               unsigned int bind_resp_wait_timeout,
                               unsigned int unbind_resp_wait_timeout);

  const std::string& getLinkSetPrefix() const;
  unsigned getTotalNumberOfOutLinks() const;

  SwitchingTable& _switchTable;

  struct OutLinkDefinition {
    OutLinkDefinition(const std::string& link_name,
                      const std::string& peer_host,
                      unsigned int peer_port,
                      unsigned int connect_timeout,
                      unsigned int bind_resp_wait_timeout,
                      unsigned int unbind_resp_wait_timeout)
    : _linkName(link_name), _peerHost(peer_host), _peerPort(peer_port),
      _connectTimeout(connect_timeout), _bindRespWaitTimeout(bind_resp_wait_timeout),
      _unbindRespWaitTimeout(unbind_resp_wait_timeout)
      {}
    std::string _linkName, _peerHost;
    unsigned int _peerPort, _connectTimeout, _bindRespWaitTimeout, _unbindRespWaitTimeout;
  };

  typedef std::vector<OutLinkDefinition> known_links_t;
  known_links_t _knownLinks;

  typedef std::set<std::string> known_link_ids_t;
  known_link_ids_t _knownLinkIds;

  smsc::logger::Logger* _logger;
private:
  std::string _linkSetPrefix;
  unsigned _totalNumberOfOutLinks;
};

}}}

#endif

