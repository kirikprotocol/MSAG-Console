/* ************************************************************************** *
 * SuaUser: SUA API implementation.
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSCCP_SCCPUSER_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SS7NA_LIBSCCP_SCCPUSER_HPP__

# include <map>
# include <netinet/in.h>

# include "logger/Logger.h"
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
  ~SccpUser();

  // ---------------------------------------------------
  // -- SccpApi interface methods
  // ---------------------------------------------------
  //Throws in case of unexpected init() call
  virtual ErrorCode_e init(const SccpConfig & sua_cfg) /*throw(std::exception)*/;

  virtual ErrorCode_e close(void);

  //NOTE: 'connect_num' is an index of LinkId from SccpConfig._links array
  virtual ErrorCode_e connect(unsigned int connect_num);

  virtual ErrorCode_e disconnect(unsigned int connect_num);

  //Instructs the SCCP provider that given connect serves (is binded to) specified SubSystems.
  //If 'conn_sccp_adr' is not NULL, returns SCCPAddress assigned to this connect by SCCP provider
  virtual ErrorCode_e bind(unsigned int connect_num, const uint8_t * ssn_list, uint8_t ssn_list_sz,
                           sccp::SCCPAddress * conn_sccp_adr = 0);

  virtual ErrorCode_e unbind(unsigned int connect_num);

  //Returns number of configured connects
  virtual unsigned int getConnectsCount(void) const;
  //Returns NULL in case of unknown 'connect_num'
  virtual ErrorCode_e getConnectInfo(LinkId & link_info, unsigned int connect_num) const;

  using SccpApi::unitdata_req;

  virtual CallResult unitdata_req(const uint8_t* message,
                                  uint16_t message_size,
                                  const uint8_t* called_addr,
                                  uint8_t called_addr_len,
                                  const uint8_t* calling_addr,
                                  uint8_t calling_addr_len,
                                  const MessageProperties& msg_properties);

  virtual ErrorCode_e unitdata_req(const uint8_t* message,
                                  uint16_t message_size,
                                  const uint8_t* called_addr,
                                  uint8_t called_addr_len,
                                  const uint8_t* calling_addr,
                                  uint8_t calling_addr_len,
                                  const MessageProperties& msg_properties,
                                  unsigned int connect_num);

  virtual ErrorCode_e msgRecv(MessageInfo* msg_info, uint32_t timeout=0);


protected:
  unsigned getConnNumByPolicy(void);

private:
  static const unsigned MAX_SOCKET_POOL_SIZE = 64;

  class LinkInputStream : public corex::io::InputStream {
  private:
    corex::io::InputStream *  _iStream;
    unsigned int              _connectNum;

  public:
    LinkInputStream(corex::io::InputStream * i_stream, unsigned int connect_num)
      : _iStream(i_stream), _connectNum(connect_num)
    { }
    ~LinkInputStream()
    { }

    unsigned int getConnectNum(void) const { return _connectNum; }
    // ------------------------------------------
    // -- InputStream interface implementation
    // ------------------------------------------
    virtual ssize_t read(uint8_t *buf, size_t buf_sz)
    {
      return _iStream->read(buf, buf_sz);
    }
    virtual ssize_t readv(const struct iovec *iov, int iovcnt)
    {
      return _iStream->readv(iov, iovcnt);
    }
    virtual corex::io::IOObject* getOwner()
    {
      return _iStream->getOwner();
    }
  };

  struct LinkInfo : public LinkId {
    corex::io::network::TCPSocket * _socket;
    LinkInputStream *               _inputStream;

    LinkInfo() : LinkId()
      , _socket(NULL), _inputStream(NULL)
    { }
    LinkInfo(const LinkId & use_link_id) : LinkId(use_link_id)
      , _socket(NULL), _inputStream(NULL)
    {  }
    ~LinkInfo()
    { }

    std::string toString(unsigned int connect_num) const;
  };

  struct CacheEntry {
    uint32_t                  expectedMessageSize;
    unsigned int              suaConnectNum;
    corex::io::InputStream *  owner;
    utilx::RingBuffer<common::TP::MAX_PACKET_SIZE> ringBuf;

    CacheEntry()
      : expectedMessageSize(0), suaConnectNum(0), owner(0)
    { }
  };

  typedef std::map<corex::io::InputStream*, CacheEntry*> packets_cache_t;

  /* -- */
  bool                      _wasInitialized;
  std::string               _appId;
  SccpConfig::TrafficMode_e _trafficMode;
  unsigned int              _lastUsedConnIdx;
  std::vector<LinkInfo>     _knownLinks;
  packets_cache_t           _packetsCache;
  CacheEntry *              _availableData;
  corex::io::IOObjectsPool  _socketPool;
  smsc::logger::Logger *    _logger;

  mutable smsc::core::synchronization::Mutex _lock;
  smsc::core::synchronization::Mutex _receiveSynchronizeLock;
  smsc::core::synchronization::Mutex _lastUsedConnIdxLock;
};

}}}

#endif /* __EYELINE_SS7NA_LIBSCCP_SCCPUSER_HPP__ */

