#include "LibSccpConnectAcceptor.hpp"
#include "LibSccpConnect.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

common::io_dispatcher::Link*
LibSccpConnectAcceptor::accept() const {
  return new LibSccpConnect(_getListenSocket()->accept(), this);
}

}}}}}
