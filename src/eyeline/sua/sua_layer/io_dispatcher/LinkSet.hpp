#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_LINKSET_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_LINKSET_HPP__

# include <list>
# include <core/synchronization/RWLock.hpp>

# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/communication/Message.hpp>
# include <eyeline/sua/communication/TP.hpp>

# include <eyeline/sua/sua_layer/io_dispatcher/Connection.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/Link.hpp>
# include <eyeline/utilx/Iterator.hpp>
# include <logger/Logger.h>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

//##Documentation
//## Класс управляет распределнием сообщений (исходящий трафик) по соединениям,
//## входящим в LinkSet, в соответствии с заданной для него политикой. Класс
//## может принимать сообщение на любом из соединений, входящих в набор.
class LinkSet : public Connection {
public:
  typedef enum {UNSPECIFIED=0, OVERRIDE=1, LOADSHARE=2, BROADCAST=3} linkset_mode_t;

  LinkSet(linkset_mode_t mode, const communication::LinkId& linkSetId);

  void addConnection(Link* connect);

  void removeConnection(Link* connect);

  virtual void send(const communication::Message& message);

  void sendBroadcast(const communication::Message& message);

  virtual communication::TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  class ConnectionIterator;
  ConnectionIterator getIterator();

  static const char* getLinksetModeStringPresentation(linkset_mode_t mode);
private:
  linkset_mode_t _mode;
  typedef std::list<Connection*> connections_lst_t;
  connections_lst_t _connections;
  connections_lst_t::iterator _lastUsedConnIter;

  smsc::core::synchronization::Mutex _currentConnLock;
  smsc::core::synchronization::RWLock _lock;
  smsc::logger::Logger* _logger;

public:
  class ConnectionIterator : public utilx::Iterator<Connection*> {
  public:
    virtual bool hasElement() const;
    virtual void next();
    virtual const_ref getCurrentElement() const;
    virtual ref getCurrentElement();
    virtual void deleteCurrentElement();
  private:
    ConnectionIterator(connections_lst_t& container, smsc::core::synchronization::RWLock& lock);
    friend class LinkSet;
    connections_lst_t& _container;
    connections_lst_t::iterator _iter, _endIter;
    smsc::core::synchronization::RWLock& _lock;
  };
};

}}}}

#endif
