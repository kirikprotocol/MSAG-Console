#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_CONNECTION_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_CONNECTION_HPP__

# include "core/synchronization/Mutex.hpp"

# include "eyeline/corex/io/network/Socket.hpp"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObjectsPool.hpp"

# include "eyeline/utilx/BoundedQueue.hpp"

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/Message.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"

# include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class ConnectAcceptor;
class ConnectMgr;

class Connection {
public:
  virtual ~Connection() {}

  virtual void send(const Message& message) = 0;

  virtual TP* receive() = 0;

  virtual bool hasReadyTransportPacket() = 0;

  virtual void bufferInputTransportPackets() = 0;

  virtual const ConnectAcceptor* getCreator() const {
    return NULL;
  }

  LinkId getLinkId() const {
    return _linkId;
  }

protected:
  void setLinkId(const LinkId& linkId) {
    _linkId = linkId;
  }

private:
  LinkId _linkId;
};

}}}}

#endif
