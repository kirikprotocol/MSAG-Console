#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_CONNECTMGR_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_CONNECTMGR_HPP__

# include <map>
# include <set>

# include <string>
# include "logger/Logger.h"
# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/EventMonitor.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/ShutdownInProgressNotification.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/Message.hpp"

# include "eyeline/ss7na/common/io_dispatcher/ConnectAcceptor.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Connection.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IOEvent.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/io_dispatcher/LinkSet.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class ConnectMgr : public utilx::ShutdownInProgressNotification {
public:
  ConnectMgr();

  void addConnectAcceptor(const std::string& acceptor_name, ConnectAcceptor* connect_acceptor);
  ConnectAcceptor* removeConnectAcceptor(const std::string& acceptor_name, bool close_all_established_connects);

  void addLink(const LinkId& link_id, Link* link);
  void addLink(const LinkId& link_id, const LinkPtr& link);
  LinkPtr removeLink(const LinkId& link_id, bool cleanup_acceptor_established_conns=true);
  bool existLink(const LinkId& link_id) const;

  void registerLinkSet(const LinkId& link_set_id, LinkSet::linkset_mode_t linkset_mode);
  void addLinkToLinkSet(const LinkId& linkset_id, const LinkId& link_id);
  LinkPtr removeLinkFromLinkSet(const LinkId& linkset_id, const LinkId& link_id);
  void removeLinkFromLinkSets(const std::set<LinkId>& linkset_ids, const LinkId& link_id);
  std::set<LinkId> getLinkSetIds(const LinkId& link_id);

  IOEvent* getEvent();

  // return LinkId used for sending of output message
  LinkId send(const LinkId& out_link_id, const Message& message);
  void sendToLinkSetBroadcast(const LinkId& out_linkset_id, const Message& output_message);

  void changeProtocolState(const LinkId& out_link_id, const Message& message);

  virtual void notifyShutdownInProgess();

protected:
  virtual IOEvent* makeConnectionReleaseEvent(const LinkPtr& closed_link) = 0;

private:
  IOEvent* readPacketAndMakeEvent(LinkPtr& link_id, bool fillup_input_buffer=false);
  IOEvent* cleanupLinkInfo(const LinkId& link_id);

  smsc::core::synchronization::Mutex _sock2link_lock, _synchronize, _connAcceptorsLock;
  mutable smsc::core::synchronization::EventMonitor _linkLock, _linkSetLock;

  typedef std::map<std::string/*name of connect acceptor*/, ConnectAcceptor*> connect_acceptors_map_t;
  connect_acceptors_map_t _connectAcceptors;

  LinkPtr _curConnect;

  typedef std::map<std::string /*key is a socket fingerprint*/, LinkPtr> socket2link_t;
  socket2link_t _socket2link;

  typedef std::map<std::string /*key is a socket fingerprint*/, ConnectAcceptor*> socket2connectAcceptor_t;
  socket2connectAcceptor_t _socket2connectAcceptor;

  enum { MAX_SOCKET_POOL_SIZE=64 };
  corex::io::IOObjectsPool _ioObjectsPool;
  int _listenStatus;
  smsc::logger::Logger* _logger;

  typedef std::map<LinkId, LinkPtr> links_t;
  links_t _activeLinks;

  typedef std::map<std::string, std::set<LinkId>* > accptrs_to_connects_t;
  accptrs_to_connects_t _acceptor2establishedConnects;

  typedef std::map<LinkId, LinkSetPtr> linksets_t;
  linksets_t _linkSets;

  typedef std::map<LinkId, std::set<LinkId>*> link_to_linksets_t;
  link_to_linksets_t _linkToLinkSets;

  bool _shutdownInProgress;

  bool sendToLinkSet(const LinkId& out_link_id, const Message& message, LinkId* used_link_id_in_linkset);
  void sendToLink(const LinkId& out_link_id, const Message& message);

  IOEvent* processReadReadyEvent(int* listen_status);
  IOEvent* createNewConnectIndicationEvent(const std::string& cur_server_socket_fingerprint);
  IOEvent* processAcceptReadyEvent(int* listen_status);
};

template<class CONNECTIONRELEASEEVENT>
class ConnectMgrTmpl : public ConnectMgr,
                       public utilx::Singleton<ConnectMgrTmpl<CONNECTIONRELEASEEVENT> > {
protected:
  ConnectMgrTmpl() {}
  friend class utilx::Singleton<ConnectMgrTmpl>;

  virtual IOEvent* makeConnectionReleaseEvent(const LinkPtr& closed_link) {
    utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_EVENT>();
    return new (ptr_desc->allocated_memory) CONNECTIONRELEASEEVENT(closed_link);
  }
};

}}}}

#endif
