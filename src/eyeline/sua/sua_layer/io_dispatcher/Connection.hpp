#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_CONNECTION_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_CONNECTION_HPP__

# include <core/synchronization/Mutex.hpp>

# include <eyeline/corex/io/network/Socket.hpp>
# include <eyeline/corex/io/IOStreams.hpp>
# include <eyeline/corex/io/IOObjectsPool.hpp>

# include <eyeline/utilx/BoundedQueue.hpp>

# include <eyeline/sua/communication/TP.hpp>
# include <eyeline/sua/communication/Message.hpp>
# include <eyeline/sua/communication/LinkId.hpp>

# include <eyeline/sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

class ConnectAcceptor;
class ConnectMgr;

class Connection {
public:
  virtual ~Connection();

  virtual void send(const communication::Message& message) = 0;

  virtual communication::TP* receive() = 0;

  virtual bool hasReadyTransportPacket() = 0;

  virtual void bufferInputTransportPackets() = 0;

  virtual const ConnectAcceptor* getCreator() const;

  communication::LinkId getLinkId() const;

protected:
  void setLinkId(const communication::LinkId& linkId);

private:
  communication::LinkId _linkId;
};

}}}}

#endif
