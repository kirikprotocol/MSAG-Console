#ifndef IO_DISPATCHER_CONNECTION_HPP_HEADER_INCLUDED_B87B31F2
# define IO_DISPATCHER_CONNECTION_HPP_HEADER_INCLUDED_B87B31F2

# include <core/synchronization/Mutex.hpp>

# include <sua/corex/io/network/Socket.hpp>
# include <sua/corex/io/IOStreams.hpp>
# include <sua/corex/io/IOObjectsPool.hpp>

# include <sua/communication/TP.hpp>
# include <sua/communication/Message.hpp>
# include <sua/communication/LinkId.hpp>

# include <sua/utilx/BoundedQueue.hpp>
# include <sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>

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

} // namespace io_dispatcher

#endif /* IO_DISPATCHER_CONNECT_HPP_HEADER_INCLUDED_B87B31F2 */
