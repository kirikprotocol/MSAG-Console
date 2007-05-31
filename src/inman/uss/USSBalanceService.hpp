#ifndef USSBALANCESERVICE_HPP_HEADER_INCLUDED_BA8AF0D8
# define USSBALANCESERVICE_HPP_HEADER_INCLUDED_BA8AF0D8

# include <core/synchronization/Mutex.hpp>
# include <inman/interaction/connect.hpp>
# include <inman/interaction/server.hpp>
# include "USSBalanceConnect.hpp"
# include <map>
# include <inman/inap/dispatcher.hpp>

# include "UssServiceCfg.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

//##ModelId=45750D40035D
class USSBalanceService  : public smsc::inman::interaction::ServerListenerITF {
public:
  USSBalanceService(const UssService_CFG& in_cfg);
  ~USSBalanceService();
  //##ModelId=45750E810169
  bool start();

  //##ModelId=45750EA80188
  void stop();

  //##ModelId=45750E5702F0
  smsc::inman::interaction::ConnectAC* onConnectOpening(smsc::inman::interaction::Server* srv,
                                                        smsc::core::network::Socket* sock);

  //##ModelId=45750E68032E
  void onConnectClosing(smsc::inman::interaction::Server* srv,
                        smsc::inman::interaction::ConnectAC* conn);

  //##ModelId=45750E720244
  void onServerShutdown(smsc::inman::interaction::Server* srv,
                        smsc::inman::interaction::Server::ShutdownReason reason);

private:
  smsc::logger::Logger*              _logger;
  UssService_CFG                     _cfg;

  //##ModelId=45750DA1006F
  smsc::inman::interaction::Server*  _server;
  smsc::core::synchronization::Mutex _mutex;

  volatile bool                      _running;

  typedef std::map<SOCKET,USSBalanceConnect*> UssConnMap_t;
  UssConnMap_t _ussConnects;

  smsc::inman::inap::TCAPDispatcher* _disp;
};

}
}
}

#endif /* USSBALANCESERVICE_HPP_HEADER_INCLUDED_BA8AF0D8 */
