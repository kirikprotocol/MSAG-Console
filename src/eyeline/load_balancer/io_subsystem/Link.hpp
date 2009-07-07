#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_LINK_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_LINK_HPP__

# include <sys/types.h>
# include <string>

# include "eyeline/load_balancer/io_subsystem/Connection.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class PacketWriter;

class Link : public Connection {
public:
  Link()
  : _packetWriter(NULL), _connectTimeout(0),
    _bindRespWaitTimeout(0), _unbindRespWaitTimeout(0)
  {}

  Link(unsigned int connect_timeout, unsigned int bind_resp_wait_timeout,
       unsigned int unbind_resp_wait_timeout)
  : _packetWriter(NULL), _connectTimeout(connect_timeout),
    _bindRespWaitTimeout(bind_resp_wait_timeout),
    _unbindRespWaitTimeout(unbind_resp_wait_timeout)
  {}

  virtual void establish() = 0;
  virtual void close() = 0;

  virtual const std::string& getPeerInfoString() const = 0;
  virtual Link* createNewOutLink() const = 0;

  void assignPacketWriter(PacketWriter* packet_writer) {
    _packetWriter = packet_writer;
  }

  const LinkId& getRelatedLinkSetId() const {
    return _relatedLinkSetId;
  }
  void resetLinkSetId() {
    _relatedLinkSetId = LinkId();
  }
  void setLinkSetId(const LinkId& link_set_id) {
    _relatedLinkSetId = link_set_id;
  }

  unsigned int getConnectTimeout() const { return _connectTimeout; }
  unsigned int getBindRespWaitTimeout() const { return _bindRespWaitTimeout; }
  unsigned int getUnbindRespWaitTimeout() const { return _unbindRespWaitTimeout; }

protected:
  PacketWriter* _packetWriter;

private:
  unsigned int _connectTimeout, _bindRespWaitTimeout, _unbindRespWaitTimeout;
  LinkId _relatedLinkSetId;
};

}}}

#endif
