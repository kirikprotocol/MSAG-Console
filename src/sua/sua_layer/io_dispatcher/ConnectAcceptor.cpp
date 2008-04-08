#include "ConnectAcceptor.hpp"

namespace io_dispatcher {

ConnectAcceptor::~ConnectAcceptor() {}

corex::io::network::ServerSocket*
ConnectAcceptor::getServerSocket() const {
  return _getListenSocket();
}

}


