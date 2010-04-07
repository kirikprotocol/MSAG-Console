#include "LibSccpConnectAcceptor.hpp"
#include "LibSccpConnect.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

common::io_dispatcher::Link*
LibSccpConnectAcceptor::accept() const {
  return new LibSccpConnect(_getListenSocket()->accept(), this);
}

}}}}
