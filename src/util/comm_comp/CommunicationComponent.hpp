#ifndef __BDB_AGENT_COMMUNICATIONCOMPONENT_HPP__
# define  __BDB_AGENT_COMMUNICATIONCOMPONENT_HPP__ 1

# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>

# include <map>
# include <string>
# include <list>
# include <sstream>
# include <logger/Logger.h>

# include <core/threads/Thread.hpp>
# include <core/network/Socket.hpp>
# include <core/synchronization/Mutex.hpp>
# include <core/synchronization/Event.hpp>

# include <util/Exception.hpp>
# include <util/Singleton.hpp>

# include <util/config/ConfigView.h>

# include <util/comm_comp/PacketFactory.hpp>
# include <util/comm_comp/NetworkExceptions.hpp>
# include <util/comm_comp/SingleSharedQueue.hpp>
# include <util/comm_comp/TEvent.hpp>

namespace smsc {
namespace util {
namespace comm_comp {

std::string getSocketFingerPrint(smsc::core::network::Socket& readySocket);
void cleanupSocketFingerPrint(smsc::core::network::Socket& readySocket);

/*
** Active class for reading/writing application objects over network. Application objects are transmitted 
** inside of transport packets. Type of transport packet, application request and response specified as template
** parameters. Class realization supports reading transport packets in parts with interim caching of incomplete
** reading packets.
** The object is received in transport packet is published to object's queue for subsequent processing.
*/
template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
class ObjectReaderWriter : public smsc::core::threads::Thread,
                           public smsc::util::Singleton<ObjectReaderWriter<TRANSPORT_PACKET, APPLICATION_REQUEST, APPLICATION_RESPONSE> > {
public:
  ObjectReaderWriter()
    : _log(NULL), _wasStopped(false), _maxFd(0) {
    FD_ZERO(&_fd_read);
    FD_ZERO(&_fd_write);
    int signal_pipe[2];
    int st = ::pipe(signal_pipe);
    if ( st < 0 )
      throw smsc::util::CustomException("ObjectReaderWriter::PacketReaderWriter::: call to pipe() failed");
    _waitNewEventFd = signal_pipe[0]; _signalNewEventFd = signal_pipe[1];

    st = ::pipe(signal_pipe);
    if ( st < 0 )
      throw smsc::util::CustomException("ObjectReaderWriter::PacketReaderWriter::: second call to pipe() failed");
    _waitShutdownEventFd = signal_pipe[0]; _signalShutdownEventFd = signal_pipe[1];
  }
  ~ObjectReaderWriter() { ::close(_waitNewEventFd); ::close(_signalNewEventFd); }

  // make class initialization. The second parameter points to opened configuraiton
  // file having parameter's section "comm_comp".
  void initialize(smsc::logger::Logger* log,
                  smsc::util::config::Manager& manager) {
    _log = log;
    _cacheOfIncompleteReadingPackets.initialize(log);
    
    _config.read(manager);
  }

  void scheduleObjectForWrite(const APPLICATION_RESPONSE& responseObject, const std::string connectId);
  virtual int Execute();
  void shutdown() {
    _wasStopped = true; close (_signalShutdownEventFd);
  }
private:
  /* configuration parameters is defined in 'comm_comp' section.
  ** This section has next parameters:
  ** <section name="comm_comp">
  **  <param name="listenHost" type="string">192.168.1.12</param>
  **  <param name="listenPort" type="int">7777</param>
  ** </section>
  */
  struct CommunicationComp_Config
  {
    CommunicationComp_Config() : listenPort(-1) {}
    std::string listenHost;
    int listenPort;

    std::string toString() {
      std::ostringstream obuf;
      obuf << "listenHost=[" << listenHost
           << "],listenPort=[" << listenPort
           << "]";
      return obuf.str();
    };
  };

  class CommunicationComp_Config_Reader : public CommunicationComp_Config {
  public:
    void read(smsc::util::config::Manager& manager) {
      if (!manager.findSection("comm_comp"))
        throw smsc::util::config::ConfigException("\'comm_comp\' section is missed");
      smsc::util::config::ConfigView dbParamCfg(manager, "comm_comp");

      listenHost = dbParamCfg.getString("listenHost");
      listenPort = dbParamCfg.getInt("listenPort");
    }
  };

  CommunicationComp_Config_Reader _config;

  APPLICATION_REQUEST* receive(smsc::core::network::Socket& readySocket);
  int processOutstandingPackets(fd_set& fd_ready_to_write);
  void preparePacketsForSending();

  template <class KEY, class T>
  class CacheOfIncompletePackets {
  public:
    CacheOfIncompletePackets() : _log(NULL) {}
    void initialize(smsc::logger::Logger* log) { _log = log; }
    T& getIncompletePacket(const KEY& key) {
      typename cacheOfIncompleteReadingPackets_t::iterator iter=_cacheOfIncompletePackets.find(key);
      if ( iter == _cacheOfIncompletePackets.end() ) {
        smsc_log_debug(_log, "SessionCache::getIncompleteMessageForSocket::: message was not found");
        std::pair<typename cacheOfIncompleteReadingPackets_t::iterator, bool> ins_res = _cacheOfIncompletePackets.insert(std::make_pair(key, new T()));
        iter = ins_res.first;
      } else
        smsc_log_debug(_log, "SessionCache::getIncompleteMessageForSocket::: message was found");

      return *(iter->second);
    }

    void removeCompletePacket(const KEY& key) {
      typename cacheOfIncompleteReadingPackets_t::iterator iter=_cacheOfIncompletePackets.find(key);
      if ( iter != _cacheOfIncompletePackets.end() ) {
        delete iter->second;
        _cacheOfIncompletePackets.erase(iter);
      }
    }
  private:
    typedef std::map<KEY, T*> cacheOfIncompleteReadingPackets_t;
    cacheOfIncompleteReadingPackets_t _cacheOfIncompletePackets;
    smsc::logger::Logger* _log;
  };

  CacheOfIncompletePackets<std::string, TRANSPORT_PACKET> _cacheOfIncompleteReadingPackets;
  smsc::logger::Logger* _log;
  volatile bool _wasStopped;
  typedef std::map<std::string, smsc::core::network::Socket*> sockets_cache_t;
  sockets_cache_t _client_sockets_cache;
  int _maxFd;
  fd_set _fd_read, _fd_write;

  smsc::core::synchronization::Mutex _scheduled_packets_for_write_lock;

  struct packet_info_t {
    packet_info_t(TRANSPORT_PACKET* pckData, const std::string& connId) : 
      packetData(pckData), connectId(connId) {}
    TRANSPORT_PACKET* packetData;
    std::string connectId; // id of connect on which packet has been received
  };

  typedef std::list<packet_info_t> scheduled_packets_t;
  scheduled_packets_t _scheduled_packets_for_write, _scheduled_packets_for_write_slice;
  int _waitNewEventFd, _signalNewEventFd;
  int _waitShutdownEventFd, _signalShutdownEventFd;

  void cleanupData();

  ObjectReaderWriter(const ObjectReaderWriter& rhs);
  ObjectReaderWriter& operator=(const ObjectReaderWriter& rhs);
};

template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
APPLICATION_REQUEST*
ObjectReaderWriter<TRANSPORT_PACKET,APPLICATION_REQUEST,APPLICATION_RESPONSE>::receive(smsc::core::network::Socket& readySocket)
{
  std::string socketFingerPrint = getSocketFingerPrint(readySocket);

  TRANSPORT_PACKET& rawPacket =
    _cacheOfIncompleteReadingPackets.getIncompletePacket(socketFingerPrint);

  if ( !rawPacket.haveReadLenght() ) {
    rawPacket.readDataLength(readySocket);
    return NULL;
  }

  if ( !rawPacket.haveReadRawData() ) {
    rawPacket.readRawData(readySocket);
    if ( !rawPacket.haveReadRawData() )
      return NULL;
  }
  //Congratulations! The message was read from network.
  smsc_log_debug(_log,"ObjectReaderWriter::receive::: Try create applicaton packet from raw data");

  APPLICATION_REQUEST::packet_code_t reqCode = 
    APPLICATION_REQUEST::extractPacketCode(rawPacket.getRawData());

  APPLICATION_REQUEST* packet = smsc::util::comm_comp::PacketFactory<APPLICATION_REQUEST>::getInstance().create(reqCode);
  packet->deserialize(rawPacket.getRawData());

  smsc_log_debug(_log,"ObjectReaderWriter::receive::: Remove packet from cache of incomplete packets");
  _cacheOfIncompleteReadingPackets.removeCompletePacket(socketFingerPrint);

  smsc_log_debug(_log,"ObjectReaderWriter::receive::: Packet was created");

  return packet;
}

template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
int 
ObjectReaderWriter<TRANSPORT_PACKET,APPLICATION_REQUEST,APPLICATION_RESPONSE>::processOutstandingPackets(fd_set& fd_ready_to_write)
{
  int numOfWrittenPackets=0;
  scheduled_packets_t::iterator out_pck_iter = _scheduled_packets_for_write_slice.begin();
  while (out_pck_iter != _scheduled_packets_for_write_slice.end()) 
  {
    sockets_cache_t::iterator sockets_iter = _client_sockets_cache.find(out_pck_iter->connectId);
    if ( sockets_iter != _client_sockets_cache.end() ) {
      smsc_log_info(_log, "ObjectReaderWriter::processOutstandingPackets::: write packet to socket with id=[%s]", out_pck_iter->connectId.c_str());
      smsc::core::network::Socket* clientSocket = sockets_iter->second;
      if ( FD_ISSET (clientSocket->getSocket(), &fd_ready_to_write) ) {
        ++numOfWrittenPackets;
        TRANSPORT_PACKET* packet = out_pck_iter->packetData;
        packet->writeDataLength(*clientSocket);
        packet->writeRawData(*clientSocket);
        if ( packet->isPacketWriteComplete() ) {
          smsc_log_info(_log, "ObjectReaderWriter::processOutstandingPackets::: packet have been written completely");
          FD_CLR(clientSocket->getSocket(), &_fd_write);
          _scheduled_packets_for_write_slice.erase(out_pck_iter++);
          delete packet; continue;
        }
      }
    }
    ++out_pck_iter;
  }
  return numOfWrittenPackets;
}

template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
void
ObjectReaderWriter<TRANSPORT_PACKET,APPLICATION_REQUEST,APPLICATION_RESPONSE>::preparePacketsForSending()
{
  uint8_t signalByte;
  if ( ::read(_waitNewEventFd, &signalByte, sizeof(signalByte)) != sizeof(signalByte) ||
       signalByte != 0 )
    throw smsc::util::CustomException("ObjectReaderWriter::preparePacketsForSending::: read from pipe failed");

  {
    smsc::core::synchronization::MutexGuard guard(_scheduled_packets_for_write_lock);
    _scheduled_packets_for_write_slice.splice(_scheduled_packets_for_write_slice.end(),_scheduled_packets_for_write);
  }
  for(scheduled_packets_t::iterator out_pck_iter = _scheduled_packets_for_write_slice.begin(); 
      out_pck_iter != _scheduled_packets_for_write_slice.end(); ++out_pck_iter) {
    sockets_cache_t::iterator sockets_iter = _client_sockets_cache.find(out_pck_iter->connectId);
    if ( sockets_iter != _client_sockets_cache.end() ) {
      smsc::core::network::Socket* clientSocket = sockets_iter->second;
      FD_SET(clientSocket->getSocket(), &_fd_write);
    }
  }
}

template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
int
ObjectReaderWriter<TRANSPORT_PACKET,APPLICATION_REQUEST,APPLICATION_RESPONSE>::Execute()
{
  try {
    if ( _config.listenHost == "" ||
         _config.listenPort < 0 ) {
      smsc_log_error(_log, "ObjectReaderWriter::Execute::: class's object isn't initialized properly");
      return -1;
    }

    smsc_log_info(_log,"ObjectReaderWriter::Execute::: try start server to accept connection for %s", _config.toString().c_str());

    smsc::core::network::Socket* srv = new smsc::core::network::Socket();
    if(srv->InitServer(_config.listenHost.c_str(),_config.listenPort,0)==-1) {
      smsc_log_error(_log, "ObjectReaderWriter::Execute::: Failed to init server socket. Terminate thread.");
      return -1;
    }

    if(srv->StartServer()==-1) {
      smsc_log_error(_log, "ObjectReaderWriter::Execute::: Failed to start server socket");
      return -1;
    }
    int srvSockFd = srv->getSocket();

    _maxFd = std::max(_waitNewEventFd, srvSockFd);
    FD_SET(_waitNewEventFd, &_fd_read );

    _maxFd = std::max(_waitShutdownEventFd, _maxFd);
    FD_SET(_waitShutdownEventFd, &_fd_read );

    FD_SET(srvSockFd, &_fd_read );

    linger l;
    l.l_onoff=1;
    l.l_linger=0;
    ::setsockopt(srv->getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));

    while(!_wasStopped)
    {
      fd_set tmp_fd_read = _fd_read, tmp_fd_write = _fd_write;
      FD_CLR(srv->getSocket(), &tmp_fd_write);

      int st = ::select(_maxFd + 1, &tmp_fd_read, &tmp_fd_write, NULL, NULL /*without timeout for a while*/);

      if ( st < 0 ) {
        smsc_log_error(_log,"ObjectReaderWriter::Execute::: select returned -1: [%s]", strerror(errno));
        return 1;
      }

      if ( _wasStopped ) {
        smsc_log_debug(_log,"ObjectReaderWriter::Execute::: component being stopping");
        break;
      }

      bool hasNewConnectionRequest = false;
      if ( st > 0 && FD_ISSET(srv->getSocket(), &tmp_fd_read) ) {
        hasNewConnectionRequest = true; --st;
      }

      if ( st > 0 && FD_ISSET(_waitNewEventFd, &tmp_fd_read) ) {
        --st; preparePacketsForSending();
      }

      if ( st > 0 ) {
        sockets_cache_t::iterator iter = _client_sockets_cache.begin(), end_iter = _client_sockets_cache.end();
        while(iter != end_iter)
        {
          if ( FD_ISSET (iter->second->getSocket(), &tmp_fd_read) ) {
            --st;
            try {
              APPLICATION_REQUEST* appObj = receive(*iter->second);
              if ( appObj ) {
                smsc_log_info(_log, "ObjectReaderWriter::Execute::: got object from network [%s]", appObj->toString().c_str());
                smsc_log_debug(_log, "ObjectReaderWriter::Execute::: push object to queue");
              
                const AbstractEvent* requestAsEvent
                  = appObj->createEvent();

                SingleSharedQueue<AcceptedObjInfo<AbstractEvent> >::getInstance().push
                  (AcceptedObjInfo<AbstractEvent>(requestAsEvent,
                                                  iter->first));
              }
            } catch (EOFException& ex) {
              char peerNameBuf[32];
              FD_CLR(iter->second->getSocket(), &_fd_read);
              cleanupSocketFingerPrint(*iter->second);
              iter->second->GetPeer(peerNameBuf); delete iter->second;
              smsc_log_info(_log, "Connection closed by peer %s", peerNameBuf);
              _client_sockets_cache.erase(iter++);
              continue;
            } catch (SocketException& ex) {
              char peerNameBuf[32];
              FD_CLR(iter->second->getSocket(), &_fd_read);
              cleanupSocketFingerPrint(*iter->second);
              iter->second->GetPeer(peerNameBuf); delete iter->second;
              smsc_log_error(_log, "Socket error on socket %s - %s", peerNameBuf, ex.what());
              _client_sockets_cache.erase(iter++);
              continue;
            }
          }
          ++iter;
        }
      }

      if ( hasNewConnectionRequest ) {
        smsc::core::network::Socket *clnt;
        clnt=srv->Accept();
        if(!clnt) {
          smsc_log_error(_log, "accept failed. error [%s] ", strerror(errno));
          return 1;
        }

        char peerNameBuf[32];
        clnt->GetPeer(peerNameBuf);
        smsc_log_info(_log, "Connection accepted from %s", peerNameBuf);
        char ptr_value_as_string[sizeof(void*)*2+1+1];
        sprintf(ptr_value_as_string,",%p",clnt);
        _client_sockets_cache.insert(std::make_pair(std::string(peerNameBuf)+std::string(ptr_value_as_string), clnt));

        _maxFd = std::max(_maxFd, clnt->getSocket());
        FD_SET(clnt->getSocket(), &_fd_read);
      }

      if ( st > 0 ) {
        st -= processOutstandingPackets(tmp_fd_write);
      }
    }
  } catch (const std::exception& ex) {
    smsc_log_error(_log, "ObjectReaderWriter::Execute::: catch unexpected exception");
  }
  cleanupData();
  return 0;
}

template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
void
ObjectReaderWriter<TRANSPORT_PACKET,APPLICATION_REQUEST,APPLICATION_RESPONSE>::cleanupData()
{
  sockets_cache_t::iterator iter = _client_sockets_cache.begin();
  sockets_cache_t::iterator end_iter = _client_sockets_cache.end();
  while (iter != end_iter) {
    delete iter->second; _client_sockets_cache.erase(iter++);
  }
}

template <class TRANSPORT_PACKET, class APPLICATION_REQUEST, class APPLICATION_RESPONSE>
void
ObjectReaderWriter<TRANSPORT_PACKET,APPLICATION_REQUEST,APPLICATION_RESPONSE>::scheduleObjectForWrite(const APPLICATION_RESPONSE& responseObject, const std::string connectId)
{
  if ( !_wasStopped ) {
    std::vector<uint8_t> responseAsRawArray;
    responseObject.serialize(responseAsRawArray);
    TRANSPORT_PACKET* rawPacketForWrite
      = new TRANSPORT_PACKET(responseAsRawArray);

    smsc::core::synchronization::MutexGuard guard(_scheduled_packets_for_write_lock);
    _scheduled_packets_for_write.push_back(packet_info_t(rawPacketForWrite, connectId));
    uint8_t signalByte=0;

    if ( _scheduled_packets_for_write.size() == 1 )
      if ( ::write(_signalNewEventFd, &signalByte, sizeof(signalByte)) != sizeof(signalByte) )
        throw smsc::util::CustomException("ObjectReaderWriter::scheduleObjectForWrite::: call write to pipe");
  }
}

}}}

#endif
