#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_CONNECTION_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_CONNECTION_HPP__

# include "eyeline/corex/io/IOObject.hpp"

# include "util/Exception.hpp"
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/load_balancer/io_subsystem/Message.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class Connection {
public:
  Connection()
    : _isSetLinkId(false) {}

  virtual ~Connection() {}

  virtual Packet* receive() = 0;
  virtual LinkId send(const Message& message) = 0;

  virtual corex::io::IOObject& getIOObject() = 0;

  const LinkId& getLinkId() const {
    if ( !_isSetLinkId )
      throw smsc::util::Exception("Connection::getLinkId::: link id isn't set");
    return _linkId;
  }

protected:
  void setLinkId(const LinkId& link_id) {
    _linkId = link_id;
    _isSetLinkId = true;
  }
  void resetLinkId() {
    _isSetLinkId = false;
  }

private:
  LinkId _linkId;
  bool _isSetLinkId;
};

}}}

#endif
