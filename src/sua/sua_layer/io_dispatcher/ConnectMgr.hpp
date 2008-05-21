#ifndef IO_DISPATCHER_CONNECTMGR_HPP_HEADER_INCLUDED_B87B0BB6
# define IO_DISPATCHER_CONNECTMGR_HPP_HEADER_INCLUDED_B87B0BB6

# include <map>
# include <set>

# include <string>
# include <sua/utilx/Singleton.hpp>
# include <core/synchronization/Mutex.hpp>
# include <core/synchronization/EventMonitor.hpp>
# include <core/buffers/RefPtr.hpp>
# include <logger/Logger.h>

# include <sua/communication/LinkId.hpp>
# include <sua/communication/Message.hpp>

# include <sua/sua_layer/io_dispatcher/ConnectAcceptor.hpp>
# include <sua/sua_layer/io_dispatcher/Connection.hpp>
# include <sua/sua_layer/io_dispatcher/IOEvent.hpp>
# include <sua/sua_layer/io_dispatcher/LinkSet.hpp>
# include <sua/utilx/Iterator.hpp>
# include <sua/utilx/ShutdownInProgressNotification.hpp>

namespace io_dispatcher {

typedef smsc::core::buffers::RefPtr<Link,smsc::core::synchronization::Mutex> LinkPtr;
typedef smsc::core::buffers::RefPtr<LinkSet,smsc::core::synchronization::Mutex> LinkSetPtr;

class ConnectMgr : public utilx::Singleton<ConnectMgr>,
                   public utilx::ShutdownInProgressNotification {
public:
  ConnectMgr();

  void addConnectAcceptor(const std::string& acceptorName, ConnectAcceptor* connectAcceptor);
  ConnectAcceptor* removeConnectAcceptor(const std::string& acceptorName);

  void addLink(const communication::LinkId& linkId, Link* link);
  LinkPtr removeLink(const communication::LinkId& linkId, bool cleanUpAcceptorEstablishedConns=true);

  void registerLinkSet(const communication::LinkId& linkSetId, LinkSet::linkset_mode_t linkSetMode);
  void addLinkToLinkSet(const communication::LinkId& linkSetId, const communication::LinkId& linkId);
  LinkPtr removeLinkFromLinkSet(const communication::LinkId& linkSetId, const communication::LinkId& linkId);
  void removeLinkFromLinkSets(const std::set<communication::LinkId>& linkSetIds, const communication::LinkId& linkId);
  std::set<communication::LinkId> getLinkSetIds(const communication::LinkId& linkId);

  IOEvent* getEvent();

  communication::LinkId send(const communication::LinkId& outLinkId, const communication::Message& message);
  void sendToLinkSetBroadcast(const communication::LinkId& outLinkSetId, const communication::Message& outputMessage);

  void changeProtocolState(const communication::LinkId& outLinkId, const communication::Message& message);

  virtual void notifyShutdownInProgess();
private:
  IOEvent* readPacketAndMakeEvent(LinkPtr& linkId, bool fillUpInputBuffer=false);
  IOEvent* cleanupLinkInfo(const communication::LinkId& linkId);

  smsc::core::synchronization::Mutex _sock2link_lock, _synchronize, _connAcceptorsLock;
  smsc::core::synchronization::EventMonitor _linkLock, _linkSetLock;

  typedef std::map<std::string/*name of connect acceptor*/, ConnectAcceptor*> connect_acceptors_map_t;
  connect_acceptors_map_t _connectAcceptors;

  LinkPtr _curConnect;

  typedef std::map<std::string /*key is a socket fingerprint*/, LinkPtr> socket2link_t;
  socket2link_t _socket2link;

  typedef std::map<std::string /*key is a socket fingerprint*/, ConnectAcceptor*> socket2connectAcceptor_t;
  socket2connectAcceptor_t _socket2connectAcceptor;

  corex::io::IOObjectsPool _ioObjectsPool;
  int _listenStatus;
  smsc::logger::Logger* _logger;

  typedef std::map<communication::LinkId, LinkPtr> links_t;
  links_t _activeLinks;

  typedef std::map<std::string, std::set<communication::LinkId>* > accptrs_to_connects_t;
  accptrs_to_connects_t _acceptor2establishedConnects;

  typedef std::map<communication::LinkId, LinkSetPtr> linksets_t;
  linksets_t _linkSets;

  typedef std::map<communication::LinkId, std::set<communication::LinkId>*> link_to_linksets_t;
  link_to_linksets_t _linkToLinkSets;

  bool _shutdownInProgress;

  bool sendToLinkSet(const communication::LinkId& outLinkId, const communication::Message& message, communication::LinkId* usedLinkIdInLinkSet);
  void sendToLink(const communication::LinkId& outLinkId, const communication::Message& message);

  IOEvent* processReadReadyEvent(int* listenStatus);
  IOEvent* createNewConnectIndicationEvent(const std::string& curServerSocketFingerPrint);
  IOEvent* processAcceptReadyEvent(int* listenStatus);
};

} // namespace io_dispatcher

#endif /* IO_DISPATCHER_CONNECTMGR_HPP_HEADER_INCLUDED_B87B0BB6 */
