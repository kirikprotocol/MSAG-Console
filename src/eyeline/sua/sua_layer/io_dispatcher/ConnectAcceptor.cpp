#include "ConnectAcceptor.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

ConnectAcceptor::~ConnectAcceptor() {}

corex::io::network::ServerSocket*
ConnectAcceptor::getServerSocket() const {
  return _getListenSocket();
}

}}}}


