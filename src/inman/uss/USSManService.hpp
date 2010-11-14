/* ************************************************************************** *
 * USSMan service: handles USS requests coming from several TCP connects in
 * asynchronous mode, manages corresponding USSD dialogs using TCAP layer.
 * ************************************************************************** */
#ifndef USSMAN_SERVICE_HPP_HEADER_INCLUDED_BA8AF0D8
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define USSMAN_SERVICE_HPP_HEADER_INCLUDED_BA8AF0D8

//# include <map>

//# include "core/synchronization/Mutex.hpp"
# include "inman/interaction/connect.hpp"
# include "inman/interaction/server.hpp"
# include "inman/inap/dispatcher.hpp"

# include "UssServiceCfg.hpp"
# include "USSManConnect.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

//##ModelId=45750D40035D
class USSManService : public smsc::inman::interaction::ServerListenerITF {
public:
  USSManService(const UssService_CFG & in_cfg, const char * log_id = NULL,
                Logger * use_log = NULL);
  ~USSManService();
  //##ModelId=45750E810169
  bool start();

  //##ModelId=45750EA80188
  void stop(bool do_wait = false);

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
  const char *                       _logId;
  UssService_CFG                     _cfg;

  //##ModelId=45750DA1006F
  smsc::inman::interaction::Server*  _server;
  smsc::core::synchronization::Mutex _mutex;

  volatile bool                      _running;

  typedef std::map<SOCKET,USSManConnect*> UssConnMap_t;
  UssConnMap_t _ussConnects;

  smsc::inman::inap::TCAPDispatcher* _disp;
};

} //uss
} //inman
} //smsc

#endif /* USSMAN_SERVICE_HPP_HEADER_INCLUDED_BA8AF0D8 */

