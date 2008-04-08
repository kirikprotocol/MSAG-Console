#ifndef IO_DISPATCHER_CONNECTACCEPTOR_HPP_HEADER_INCLUDED_B87B62DD
# define IO_DISPATCHER_CONNECTACCEPTOR_HPP_HEADER_INCLUDED_B87B62DD

# include <sua/corex/io/network/Socket.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/sua_layer/io_dispatcher/Link.hpp>
# include <sua/sua_layer/io_dispatcher/IOEvent.hpp>

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

} // namespace io_dispatcher

#endif /* IO_DISPATCHER_CONNECTACCEPTOR_HPP_HEADER_INCLUDED_B87B62DD */
