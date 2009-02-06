#ifndef __SMPPDMPLX_SESSIONHELPER__HPP__
# define __SMPPDMPLX_SESSIONHELPER__HPP__

# include <string>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

class SessionHelper {
public:
  static void terminateSessionToSme(smsc::core_ax::network::Socket& socketToSme);
  static void terminateSessionToSmsc(smsc::core_ax::network::Socket& socketToSmsc, const std::string& systemId);
  static void dropActiveSession(smsc::core_ax::network::Socket& socket);
};

}

#endif
