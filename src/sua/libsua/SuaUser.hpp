#ifndef __SUA_LIBSUA_SUAUSER_HPP__
# define __SUA_LIBSUA_SUAUSER_HPP__ 1

# include <map>
# include <logger/Logger.h>
# include <sua/libsua/SuaApi.hpp>
# include <util/config/ConfigView.h>
# include <netinet/in.h>
# include <sua/corex/io/network/TCPSocket.hpp>
# include <sua/corex/io/IOObjectsPool.hpp>
# include <sua/utilx/RingBuffer.hpp>
# include <core/synchronization/Mutex.hpp>
# include <sua/communication/TP.hpp>

namespace libsua {

class SuaUser : public SuaApi {
public:
  SuaUser(/*smsc::util::config::ConfigView* config*/);

  virtual void sua_init(smsc::util::config::ConfigView* config);

  virtual void sua_close();

  virtual void sua_connect(unsigned int suaConnectNum);

  virtual void sua_disconnect(unsigned int suaConnectNum);

  virtual unsigned int bind(unsigned int suaConnectNum);

  virtual void unbind(unsigned int suaConnectNum);

  virtual unsigned int unitdata_req(const uint8_t* message,
                                    uint16_t messageSize,
                                    const uint8_t* calledAddr,
                                    uint8_t calledAddrLen,
                                    const uint8_t* callingAddr,
                                    uint8_t callingAddrLen,
                                    const MessageProperties& msgProperties,
                                    unsigned int suaConnectNum);

  virtual void msgRecv(MessageInfo* msgInfo);

  virtual unsigned int sua_getConnectsCount() const;
protected:
  virtual unsigned int getConnNumByPolicy();
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
      : ringBuf(new utilx::RingBuffer<communication::TP::MAX_PACKET_SIZE>()), expectedMessageSize(0), suaConnectNum(0) {}
    ~CacheEntry() { delete ringBuf; }

    utilx::RingBuffer<communication::TP::MAX_PACKET_SIZE>* ringBuf;
    size_t expectedMessageSize;
    unsigned int suaConnectNum;
  };

  typedef std::map<corex::io::InputStream*, CacheEntry> packets_cache_t;
  packets_cache_t _packetsCache;

  bool _wasInitialized;
  std::string _appId, _trafficMode;
  unsigned int _hopCountValue;

  mutable smsc::core::synchronization::Mutex _lock;
  smsc::core::synchronization::Mutex _receiveSynchronizeLock;

  std::vector<LinkInfo> _knownLinks;
  smsc::logger::Logger* _logger;

  corex::io::IOObjectsPool _socketPool;
};

}

#endif
