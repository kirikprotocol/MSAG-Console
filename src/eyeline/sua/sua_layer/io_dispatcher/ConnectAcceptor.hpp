#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_CONNECTACCEPTOR_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_CONNECTACCEPTOR_HPP__

# include <eyeline/corex/io/network/Socket.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/Link.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/IOEvent.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

class ConnectAcceptor {
public:
  virtual ~ConnectAcceptor();
  virtual Link* accept() const = 0;

  // get name of ConnectAcceptor object
  virtual const std::string& getName() const = 0;

  virtual IOEvent* createIOEvent(const communication::LinkId& linkId) const = 0;
protected:
  friend class ConnectMgr; // to grant access to getSocket()

  corex::io::network::ServerSocket* getServerSocket() const;

  virtual corex::io::network::ServerSocket* _getListenSocket() const = 0;
};

}}}}

#endif
