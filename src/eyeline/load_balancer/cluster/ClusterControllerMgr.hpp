#ifndef __EYELINE_LOADBALANCER_CLUSTER_CLUSTERCONTROLLERMGR_HPP__
# define __EYELINE_LOADBALANCER_CLUSTER_CLUSTERCONTROLLERMGR_HPP__

# include <netinet/in.h>
# include <string>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "eyeline/corex/io/network/TCPSocket.hpp"
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/load_balancer/cluster/ClusterLoadBalancerProtocolClnt.hpp"
# include "eyeline/load_balancer/cluster/ClusterControllerMsgsHndlr.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

class ClusterControllerMgr : public smsc::core::threads::Thread {
public:
  ClusterControllerMgr(const std::string& peer_host,
                       in_port_t peer_port,
                       unsigned connect_timeout,
                       unsigned connect_attempt_period);

  virtual ~ClusterControllerMgr();

  virtual int Execute();

  void startup();
  void shutdown();

  void writeBuffer(const char* bufferForWrite, size_t bytesToWrite);
protected:
  void tryConnect();

  void readPacket(io_subsystem::Packet* packet);
  void processCommand(const io_subsystem::Packet& packet);
  void waitForPeriod(unsigned period);
  void readn(corex::io::InputStream* istream, uint8_t* data, size_t datalen);
  void writen(corex::io::OutputStream* ostream, const uint8_t* databuf, size_t databuf_len);

private:
  smsc::logger::Logger* _logger;
  bool _shutdownInProgress, _connected;

  corex::io::network::TCPSocket* _socket;

  std::string _peerHost;
  in_port_t _peerPort;
  unsigned _connectTimeout, _connectAttemptPeriod;
  ClusterLoadBalancerProtocol _protocolMessagesDispatcher;
  ClusterControllerMsgsHndlr _protocolMessagesHandler;
};

}}}

#endif
