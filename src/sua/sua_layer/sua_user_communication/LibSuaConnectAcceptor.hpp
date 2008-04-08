#ifndef SUA_USER_COMMUNICATION_LIBSUACONNECTACCEPTOR_HPP_HEADER_INCLUDED_B87B6208
# define SUA_USER_COMMUNICATION_LIBSUACONNECTACCEPTOR_HPP_HEADER_INCLUDED_B87B6208

# include <string>
# include <sua/sua_layer/io_dispatcher/ConnectAcceptor.hpp>
# include <sua/corex/io/network/TCPServerSocket.hpp>
# include <sua/communication/LinkId.hpp>

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

} // namespace sua_user_communication



#endif /* SUA_USER_COMMUNICATION_LIBSUACONNECTACCEPTOR_HPP_HEADER_INCLUDED_B87B6208 */
