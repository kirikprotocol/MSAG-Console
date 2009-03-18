#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_LINK_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_LINK_HPP__

# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>

# include <eyeline/corex/io/network/Socket.hpp>
# include <eyeline/corex/io/IOStreams.hpp>
# include <eyeline/corex/io/IOObjectsPool.hpp>
# include <eyeline/utilx/BoundedQueue.hpp>

# include <eyeline/sua/communication/TP.hpp>
# include <eyeline/sua/communication/Message.hpp>

# include <eyeline/sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/Connection.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
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
  explicit Link(ProtocolState* initialState);

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
  smsc::logger::Logger* _logger;
};

}}}}

#endif
