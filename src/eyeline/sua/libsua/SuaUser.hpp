#ifndef __EYELINE_SUA_LIBSUA_SUAUSER_HPP__
# define __EYELINE_SUA_LIBSUA_SUAUSER_HPP__

# include <map>
# include <sys/types.h>
# include <netinet/in.h>
# include <logger/Logger.h>
# include <util/config/ConfigView.h>
# include <core/synchronization/Mutex.hpp>

# include <eyeline/corex/io/network/TCPSocket.hpp>
# include <eyeline/corex/io/IOObjectsPool.hpp>
# include <eyeline/utilx/RingBuffer.hpp>
# include <eyeline/sua/libsua/SuaApi.hpp>
# include <eyeline/sua/communication/TP.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class SuaUser : public SuaApi {
public:
  SuaUser();

  virtual int sua_init(smsc::util::config::ConfigView* config);

  virtual int sua_close();

  virtual int sua_connect(unsigned int suaConnectNum);

  virtual int sua_disconnect(unsigned int suaConnectNum);

  virtual int bind(unsigned int suaConnectNum);

  virtual int unbind(unsigned int suaConnectNum);

  virtual int unitdata_req(const uint8_t* message,
                           uint16_t messageSize,
                           const uint8_t* calledAddr,
                           uint8_t calledAddrLen,
                           const uint8_t* callingAddr,
                           uint8_t callingAddrLen,
                           const MessageProperties& msgProperties,
                           unsigned int suaConnectNum);

  virtual int msgRecv(MessageInfo* msgInfo, uint32_t timeout=0);

  virtual size_t sua_getConnectsCount() const;
protected:
  virtual int getConnNumByPolicy();
private:
  typedef enum { NOT_CONNECTED, CONNECTED, BINDED } connection_state_t;

  class LinkInputStream : public corex::io::InputStream {
  public:
    LinkInputStream(corex::io::InputStream* iStream, unsigned int connectNum);
    virtual ssize_t read(uint8_t *buf, size_t bufSz);
    virtual corex::io::IOObject* getOwner();
    unsigned int getConnectNum() const;
  private:
    corex::io::InputStream* _iStream;
    unsigned int _connectNum;
  };

  struct LinkInfo {
    LinkInfo();
    LinkInfo(const std::string& aLinkName, const std::string& aSuaLayerHost, in_port_t aSuaLayerPort);

    std::string toString() const;

    std::string linkName;
    std::string suaLayerHost;
    in_port_t suaLayerPort;
    corex::io::network::TCPSocket* socket;
    connection_state_t connectionState;
    LinkInputStream* inputStream;
  };

  struct CacheEntry {
    CacheEntry()
      : expectedMessageSize(0), suaConnectNum(0) {}

    utilx::RingBuffer<communication::TP::MAX_PACKET_SIZE> ringBuf;
    uint32_t expectedMessageSize;
    unsigned int suaConnectNum;
    corex::io::InputStream* owner;
  };

  typedef std::map<corex::io::InputStream*, CacheEntry*> packets_cache_t;
  packets_cache_t _packetsCache;
  
  CacheEntry* availableData;

  bool _wasInitialized;

  typedef enum {OVERRIDE=1, LOADSHARE=2} traffic_mode_t;

  std::string _appId;
  traffic_mode_t _trafficMode;

  unsigned int _lastUsedConnIdx;

  mutable smsc::core::synchronization::Mutex _lock;
  smsc::core::synchronization::Mutex _receiveSynchronizeLock;
  smsc::core::synchronization::Mutex _lastUsedConnIdxLock;

  std::vector<LinkInfo> _knownLinks;
  smsc::logger::Logger* _logger;

  corex::io::IOObjectsPool _socketPool;

  traffic_mode_t convertStringToTrafficModeValue(const std::string& trafficMode,
                                                 const std::string& where);
};

}}}

#endif
