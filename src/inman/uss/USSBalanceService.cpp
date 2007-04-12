#include <assert.h>

#include "USSBalanceService.hpp"
#include <logger/Logger.h>
#include "ussmessages.hpp"

namespace smsc  {
namespace inman {
namespace uss {

USSBalanceService::USSBalanceService(const UssService_CFG& in_cfg)
  : _logger(Logger::getInstance("smsc.uss.BalanceService")), _cfg(in_cfg),
    _server(new smsc::inman::interaction::Server(&_cfg.sock, _logger)),
    _running(false),
    _disp(smsc::inman::inap::TCAPDispatcher::getInstance())
{
  assert(_server && _disp);

  smsc_log_debug(_logger, "UssBalanceService: Creating ..");

  _cfg.ss7.userId += 39; //adjust USER_ID to PortSS7 units id
  if (!_disp->connect(_cfg.ss7.userId))
    smsc_log_error(_logger, "UssBalanceService: EINSS7 stack unavailable!!!");
  else {
    smsc_log_debug(_logger, "UssBalanceService: TCAP dispatcher has connected to SS7 stack");
    if (!_disp->openSSN(_cfg.ss7.own_ssn, _cfg.ss7.maxDlgId))
      smsc_log_error(_logger, "UssBalanceService: SSN[%u] unavailable!!!", _cfg.ss7.own_ssn);
  }

  _server->addListener(this);
  smsc_log_debug(_logger, "UssBalanceService: TCP server inited");
}

USSBalanceService::~USSBalanceService()
{
  UssConnMap_t::iterator iter=_ussConnects.begin();
  while (iter!=_ussConnects.end()) {
    delete iter->second;
    _ussConnects.erase(iter++);
  }
}

//##ModelId=45750E810169
bool USSBalanceService::start()
{
  if (!_running) {
    if (!_server->Start())
      return false;

    _running = true;
    smsc_log_debug(_logger, "UssBalanceService: Started.");
  }
  return _running;
}

//##ModelId=45750EA80188
void USSBalanceService::stop()
{
  if (_running) {
    smsc_log_debug(_logger, "USSBalanceService: Stopping TCP server ..");
    _server->Stop();
  }

  _disp->Stop();
  _running = false;
  smsc_log_debug(_logger, "USSBalanceService: Stopped.");
}

//##ModelId=45750E5702F0
smsc::inman::interaction::ConnectAC*
USSBalanceService::onConnectOpening(smsc::inman::interaction::Server* srv,
                                    smsc::core::network::Socket* sock)
{
  std::auto_ptr<smsc::inman::interaction::Connect>
    conn(new smsc::inman::interaction::Connect(sock,
                                               smsc::inman::interaction::SerializerUSS::getInstance()));

  smsc_log_debug(_logger, "USSBalanceService::onConnectOpened::: got new connection request on socket=%d", sock->getSocket());
  USSBalanceConnect *ussCon = new USSBalanceConnect(_logger, _cfg);
  if (ussCon) {
    smsc::core::synchronization::MutexGuard guard(_mutex);
    _ussConnects.insert(UssConnMap_t::value_type(sock->getSocket(), ussCon));
    conn->addListener(ussCon);

    smsc_log_debug(_logger, "USSBalanceService: New USSBalanceConnect was created");
  }
  return conn.release();
}

//##ModelId=45750E68032E
void
USSBalanceService::onConnectClosing(smsc::inman::interaction::Server* srv,
                                    smsc::inman::interaction::ConnectAC* conn)
{
  SOCKET sockId = conn->getSocket()->getSocket();
  smsc_log_debug(_logger, "USSBalanceService::onConnectClosing::: close connection on socket=%d", sockId);
  smsc::core::synchronization::MutexGuard guard(_mutex);

  UssConnMap_t::iterator it = _ussConnects.find(sockId);
  if (it != _ussConnects.end()) {
    USSBalanceConnect *ussCon = it->second;
    (static_cast<smsc::inman::interaction::Connect*>(conn))->removeListener(ussCon);
    _ussConnects.erase(it);

    delete ussCon;
    smsc_log_info(_logger, "USSBalanceService: USSBalanceConnect[%u] being closed", sockId);
  } else
    smsc_log_warn(_logger, "USSBalanceService: attempt to close unknown connect[%u]", sockId);

}

//##ModelId=45750E720244
void
USSBalanceService::onServerShutdown(smsc::inman::interaction::Server* srv,
                                    smsc::inman::interaction::Server::ShutdownReason reason)
{
  smsc_log_debug(_logger, "USSBalanceService: TCP server shutdowning, reason %d", reason);
  srv->removeListener(this);

  if (reason != smsc::inman::interaction::Server::srvStopped) //abnormal shutdown
    throw CustomException("USSBalanceService: TCP server fatal failure, exiting.");

}

}
}
}
