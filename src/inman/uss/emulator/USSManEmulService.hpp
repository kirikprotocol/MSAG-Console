#ifndef __INMAN_USS_EMULATOR_USSMANEMULSERVICE_HPP__
# define __INMAN_USS_EMULATOR_USSMANEMULSERVICE_HPP__

# include "inman/interaction/connect.hpp"
# include "inman/interaction/server.hpp"
# include "util/config/ConfigView.h"
# include "USSManEmulConnect.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

using smsc::logger::Logger;

class USSManEmulService : public smsc::inman::interaction::ServerListenerITF {
public:
  USSManEmulService(const inman::interaction::ServSocketCFG& servSockCfg,
                    const char * log_id = NULL,
                    Logger * use_log = NULL);
  ~USSManEmulService();

  bool start();

  void stop(bool do_wait = false);

  smsc::inman::interaction::ConnectAC* onConnectOpening(interaction::Server* srv,
                                                        core::network::Socket* sock);

  void onConnectClosing(interaction::Server* srv,
                        interaction::ConnectAC* conn);

  void onServerShutdown(interaction::Server* srv,
                        interaction::Server::ShutdownReason reason);

private:
  smsc::logger::Logger* _logger;
  const char* _logId;

  smsc::inman::interaction::Server* _server;
  smsc::core::synchronization::Mutex _mutex;

  volatile bool _running;

  typedef std::map<SOCKET,USSManEmulConnect*> UssConnMap_t;
  UssConnMap_t _ussConnects;
};

} //uss
} //inman
} //smsc

#endif
