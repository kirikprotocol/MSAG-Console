#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LIBSUACONNECTACCEPTOR_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LIBSUACONNECTACCEPTOR_HPP__

# include <string>
# include <eyeline/corex/io/network/TCPServerSocket.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ConnectAcceptor.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

class LibSuaConnectAcceptor : public io_dispatcher::ConnectAcceptor {
public:
  LibSuaConnectAcceptor(const std::string& acceptorName, const std::string& host, in_port_t listenPort);
  virtual ~LibSuaConnectAcceptor();

  virtual io_dispatcher::Link* accept() const;

  virtual const std::string& getName() const;

  virtual io_dispatcher::IOEvent* createIOEvent(const communication::LinkId& linkId) const;

protected:
  virtual corex::io::network::ServerSocket* _getListenSocket() const;

private:
  corex::io::network::TCPServerSocket* _serverSocket;

  const std::string _acceptorName;
};

}}}}

#endif
