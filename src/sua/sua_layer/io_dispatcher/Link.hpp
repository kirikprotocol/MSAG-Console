#ifndef __SUA_SUALAYER_IODISPATCHER_LINK_HPP__
# define __SUA_SUALAYER_IODISPATCHER_LINK_HPP__

# include <core/synchronization/Mutex.hpp>

# include <sua/corex/io/network/Socket.hpp>
# include <sua/corex/io/IOStreams.hpp>
# include <sua/corex/io/IOObjectsPool.hpp>

# include <sua/communication/TP.hpp>
# include <sua/communication/Message.hpp>
# include <sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>

# include <sua/utilx/BoundedQueue.hpp>
# include <sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>
# include <sua/sua_layer/io_dispatcher/Connection.hpp>

namespace io_dispatcher {

class ConnectMgr;
class LinkSet;

class Link : public Connection {
public:
  virtual ~Link();

  virtual void send(const communication::Message& message);

  virtual communication::TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  void setWhole(LinkSet* whole);

  LinkSet* getWhole() const;

protected:
  Link(ProtocolState* initialState);

  void changeProtocolState(const communication::Message& message);
  void changeProtocolState(const IndicationPrimitive& indication);

  corex::io::network::Socket* getSocket();

  virtual corex::io::network::Socket* _getSocket() const = 0;

  friend class ConnectMgr; // to grant access to getSocket();

  virtual corex::io::OutputStream* getOutputStream() const = 0;

  void genericSend(corex::io::OutputStream*, const communication::TP* transportPacket);

private:
  smsc::core::synchronization::Mutex _protStateCtrlLock;
  io_dispatcher::ProtocolStateController _protocolStateCtrl;

  void sendToStream(corex::io::OutputStream* oStream, const communication::TP* packetToSend);

  smsc::core::synchronization::Mutex _lock;
  utilx::BoundedQueue<const communication::TP*> _queue;
  volatile bool _flag;

  LinkSet* _whole;
};

} // namespace io_dispatcher

#endif /* IO_DISPATCHER_CONNECT_HPP_HEADER_INCLUDED_B87B31F2 */
