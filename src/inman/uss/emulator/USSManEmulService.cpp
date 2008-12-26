#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <assert.h>
#include <inman/uss/ussmessages.hpp>
#include "USSManEmulService.hpp"

using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace uss {

USSManEmulService::USSManEmulService(const inman::interaction::ServSocketCFG& servSockCfg,
                                     const char * log_id/* = NULL*/,
                                     Logger * use_log/* = NULL*/)
  : _logger(use_log ? use_log : Logger::getInstance("smsc.ussman.Service")),
    _logId(log_id ? log_id : "USSManSrv"),
    _server(new interaction::Server(&servSockCfg, _logger)),
    _running(false)
{
  assert(_server);
  smsc_log_debug(_logger, "%s: Creating ..", _logId);

  _server->addListener(this);

  smsc_log_debug(_logger, "%s: TCP server inited", _logId);
}

USSManEmulService::~USSManEmulService()
{
  stop(true);
  MutexGuard guard(_mutex);
  UssConnMap_t::iterator iter=_ussConnects.begin();
  while (iter!=_ussConnects.end()) {
    delete iter->second;
    _ussConnects.erase(iter++);
  }
}

bool
USSManEmulService::start()
{
  MutexGuard guard(_mutex);
  if (_running)
    return true;

  if (!_server->Start()) {
    smsc_log_error(_logger, "%s: TCPSrv startup failure", _logId);
    return false;
  }
  smsc_log_debug(_logger, "%s: Started.", _logId);
  return _running = true;
}

void
USSManEmulService::stop(bool do_wait/* = false*/)
{
  {
    MutexGuard guard(_mutex);
    if (!_running && !do_wait)
      return;
  }
  //notify threads about stopping
  smsc_log_debug(_logger, "%s: Stopping TCP server ..", _logId);
  _server->Stop(0); //after that ConnectListenerITF methods are called
  smsc_log_debug(_logger, "%s: Stopping TCAP dispatcher ..", _logId);
  {
    MutexGuard guard(_mutex);
    _running = false;
  }
  if (do_wait) { //wait for threads
    _server->Stop(); //after that ConnectListenerITF methods are called
    smsc_log_debug(_logger, "%s: Stopped.", _logId);
  }
}

smsc::inman::interaction::ConnectAC*
USSManEmulService::onConnectOpening(interaction::Server* srv,
                                    core::network::Socket* sock)
{
  std::auto_ptr<interaction::Connect>
    conn(new interaction::Connect(sock, interaction::SerializerUSS::getInstance(),
                                  _logger));

  smsc_log_debug(_logger, "%s::onConnectOpened: got new connection request on socket=%d",
                 _logId, sock->getSocket());
  USSManEmulConnect *ussCon = new USSManEmulConnect(_logger);
  if (ussCon) {
    core::synchronization::MutexGuard guard(_mutex);
    _ussConnects.insert(UssConnMap_t::value_type(sock->getSocket(), ussCon));
    conn->addListener(ussCon);

    smsc_log_debug(_logger, "%s: New Connect was created", _logId);
  }
  return conn.release();
}

void
USSManEmulService::onConnectClosing(interaction::Server* srv,
                                    interaction::ConnectAC* conn)
{
  SOCKET sockId = conn->getSocket()->getSocket();
  smsc_log_debug(_logger, "%s::onConnectClosing: close connection on socket=%d", _logId, sockId);
  core::synchronization::MutexGuard guard(_mutex);

  UssConnMap_t::iterator it = _ussConnects.find(sockId);
  if (it != _ussConnects.end()) {
    USSManEmulConnect *ussCon = it->second;
    (static_cast<interaction::Connect*>(conn))->removeListener(ussCon);
    _ussConnects.erase(it);
    ussCon->close(conn);
    delete ussCon;
    smsc_log_info(_logger, "%s: Connect[%u] being closed", _logId, sockId);
  } else
    smsc_log_warn(_logger, "$s: attempt to close unknown connect[%u]", _logId, sockId);

}

void
USSManEmulService::onServerShutdown(interaction::Server* srv,
                                    interaction::Server::ShutdownReason reason)
{
  smsc_log_debug(_logger, "%s: TCP server shutdowning, reason %d", _logId, reason);
  srv->removeListener(this);
}

} //uss
} //inman
} //smsc

