/* ************************************************************************** *
 * SuaUser: SUA API implementation.
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSCCP_SCCPUSER_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SS7NA_LIBSCCP_SCCPUSER_HPP__

# include <map>
# include <netinet/in.h>

# include "logger/Logger.h"
# include "util/config/ConfigView.h"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/libsccp/SccpApi.hpp"
# include "eyeline/corex/io/network/TCPSocket.hpp"
# include "eyeline/corex/io/IOObjectsPool.hpp"
# include "eyeline/utilx/RingBuffer.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class SccpUser : public SccpApi {
public:
  SccpUser();

  virtual ErrorCode_e init(smsc::util::config::ConfigView* config);

  virtual ErrorCode_e close();

  virtual ErrorCode_e connect(unsigned int connect_num);

  virtual ErrorCode_e disconnect(unsigned int connect_num);

  virtual ErrorCode_e bind(unsigned int connect_num, uint8_t* ssn_list, uint8_t ssn_list_sz);

  virtual ErrorCode_e unbind(unsigned int connect_num);

  using SccpApi::unitdata_req;

  virtual CallResult unitdata_req(const uint8_t* message,
                                  uint16_t message_size,
                                  const uint8_t* called_addr,
                                  uint8_t called_addr_len,
                                  const uint8_t* calling_addr,
                                  uint8_t calling_addr_len,
                                  const MessageProperties& msg_properties);

  virtual CallResult unitdata_req(const uint8_t* message,
                                  uint16_t message_size,
                                  const uint8_t* called_addr,
                                  uint8_t called_addr_len,
                                  const uint8_t* calling_addr,
                                  uint8_t calling_addr_len,
                                  const MessageProperties& msg_properties,
                                  unsigned int connect_num);

  virtual ErrorCode_e msgRecv(MessageInfo* msg_info, uint32_t timeout=0);

  virtual unsigned getConnectsCount() const;
protected:
  virtual unsigned getConnNumByPolicy();

private:
  enum LinkState_e { linkNOT_CONNECTED, linkCONNECTED, linkBINDED };

  class LinkInputStream : public corex::io::InputStream {
  public:
    LinkInputStream(corex::io::InputStream* i_stream, unsigned int connect_num);
    virtual ssize_t read(uint8_t *buf, size_t buf_sz);
    virtual ssize_t readv(const struct iovec *iov, int iovcnt);
    virtual corex::io::IOObject* getOwner();
    unsigned int getConnectNum() const;
  private:
    corex::io::InputStream* _iStream;
    unsigned int _connectNum;
  };

  struct LinkInfo {
    LinkInfo();
    LinkInfo(const std::string& link_name, const std::string& host, in_port_t port);

    std::string toString() const;

    std::string linkName;
    std::string host;
    in_port_t port;
    corex::io::network::TCPSocket* socket;
    LinkState_e      connectionState;
    LinkInputStream* inputStream;
  };

  struct CacheEntry {
    CacheEntry()
      : expectedMessageSize(0), suaConnectNum(0) {}

    utilx::RingBuffer<common::TP::MAX_PACKET_SIZE> ringBuf;
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

  enum {MAX_SOCKET_POOL_SIZE=64};
  corex::io::IOObjectsPool _socketPool;

  traffic_mode_t convertStringToTrafficModeValue(const std::string& traffic_mode,
                                                 const std::string& where);
};

}}}

#endif

