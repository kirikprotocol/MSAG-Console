#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_LINKSET_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_LINKSET_HPP__

# include <list>

# include "logger/Logger.h"
# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/RWLock.hpp"

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/Message.hpp"

# include "eyeline/ss7na/common/io_dispatcher/Connection.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class LinkSet : public Connection {
public:
  typedef enum {UNSPECIFIED=0, OVERRIDE=1, LOADSHARE=2, BROADCAST=3} linkset_mode_t;

  LinkSet(linkset_mode_t mode, const LinkId& linkSetId);

  void addConnection(Link* connect);

  void removeConnection(Link* connect);

  virtual void send(const Message& message);

  void sendBroadcast(const Message& message);

  virtual TP* receive();

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
  class ConnectionIterator {
  public:
    ~ConnectionIterator();
    bool hasElement() const;
    void next();
    const Connection* getCurrentElement() const;
    Connection* getCurrentElement();
    void deleteCurrentElement();
  private:
    ConnectionIterator(connections_lst_t& container, smsc::core::synchronization::RWLock& lock);
    friend class LinkSet;
    connections_lst_t& _container;
    connections_lst_t::iterator _iter, _endIter;
    smsc::core::synchronization::RWLock& _lock;
  };
};

typedef smsc::core::buffers::RefPtr<LinkSet, smsc::core::synchronization::Mutex> LinkSetPtr;

}}}}

#endif
