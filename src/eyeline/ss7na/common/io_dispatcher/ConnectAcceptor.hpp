#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_CONNECTACCEPTOR_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_CONNECTACCEPTOR_HPP__

# include "eyeline/corex/io/network/Socket.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IOEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class ConnectAcceptor {
public:
  virtual ~ConnectAcceptor() {}
  virtual Link* accept() const = 0;

  virtual const std::string& getName() const = 0;

  virtual IOEvent* createIOEvent(const LinkId& linkId) const = 0;
protected:
  friend class ConnectMgr; // to grant access to getSocket()

  corex::io::network::ServerSocket* getServerSocket() const {
    return _getListenSocket();
  }

  virtual corex::io::network::ServerSocket* _getListenSocket() const = 0;
};

}}}}

#endif
