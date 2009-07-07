#include "util/Exception.hpp"
#include "eyeline/utilx/strtol.hpp"

#include "LinkId.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

/*

LinkId::LinkId(const std::string& linkIdValue)
  : _isSetListeningIface(false), _listeningPort(0)
{
  std::string::size_type idx = linkIdValue.rfind('.');
  if ( idx != std::string::npos ) {
    //throw smsc::util::Exception("LinkId::LinkId::: argument value [%s] invalid", linkIdValue.c_str());
    _listeningIface = linkIdValue.substr(0, idx);
    const std::string& portStr = linkIdValue.substr(idx+1);
    _listeningPort = static_cast<in_port_t>(utilx::strtol(portStr.c_str(), NULL, 10));
    if ( !_listeningPort && errno == ERANGE )
      throw smsc::util::Exception("LinkId::LinkId::: argument value [%s] invalid - invalid port", linkIdValue.c_str());
    _isSetListeningIface = true;
  } else {
    _listeningIface = linkIdValue;
  }
}
*/

}}};
