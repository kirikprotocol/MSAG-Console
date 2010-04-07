#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_LINK_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_LINK_HPP__

# include "logger/Logger.h"
# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/corex/io/network/Socket.hpp"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObjectsPool.hpp"
# include "eyeline/utilx/BoundedQueue.hpp"

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/Message.hpp"

# include "eyeline/ss7na/common/io_dispatcher/Connection.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IndicationPrimitive.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class ConnectMgr;
class LinkSet;

class Link : public Connection {
public:
  virtual ~Link();

  virtual void send(const Message& message);

  virtual TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  void setWhole(LinkSet* whole);

  LinkSet* getWhole() const;

protected:
  explicit Link(ProtocolState* initial_state);

  void changeProtocolState(const Message& message);
  void changeProtocolState(const IndicationPrimitive& indication);

  corex::io::network::Socket* getSocket() const;

  virtual corex::io::network::Socket* _getSocket() const = 0;

  friend class ConnectMgr; // to grant access to getSocket();

  virtual corex::io::OutputStream* getOutputStream() const = 0;

  void genericSend(corex::io::OutputStream* o_stream, const TP* transport_packet);

private:
  smsc::core::synchronization::Mutex _protStateCtrlLock;
  io_dispatcher::ProtocolStateController _protocolStateCtrl;

  void sendToStream(corex::io::OutputStream* o_stream, const TP* packet_to_send);

  smsc::core::synchronization::Mutex _lock;
  utilx::BoundedQueue<const TP*> _queue;
  volatile bool _flag;

  LinkSet* _whole;
  smsc::logger::Logger* _logger;
};

typedef smsc::core::buffers::RefPtr<Link, smsc::core::synchronization::Mutex> LinkPtr;

}}}}

#endif
