#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>

#include "USSManService.hpp"
#include "ussmessages.hpp"

#include "inman/comp/map_uss/MapUSSFactory.hpp"
using smsc::inman::comp::_ac_map_networkUnstructuredSs_v2;
using smsc::inman::comp::uss::initMAPUSS2Components;

using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace uss {

USSManService::USSManService(const UssService_CFG & in_cfg, const char * log_id/* = NULL*/,
                             Logger * use_log/* = NULL*/)
  : _logger(use_log ? use_log : Logger::getInstance("smsc.ussman.Service"))
  , _logId(log_id ? log_id : "USSManSrv"), _cfg(in_cfg)
  , _server(new smsc::inman::interaction::Server(&_cfg.sock, _logger))
  , _running(false), _disp(new smsc::inman::inap::TCAPDispatcher())
{
    assert(_server && _disp);
    smsc_log_debug(_logger, "%s: Creating ..", _logId);

    _disp->Init(_cfg.ss7, _logger);
    _cfg.tcDisp = _disp;
    _server->addListener(this);

    smsc_log_debug(_logger, "%s: TCP server inited", _logId);
}

USSManService::~USSManService()
{
    stop(true);
    MutexGuard guard(_mutex);
    UssConnMap_t::iterator iter=_ussConnects.begin();
    while (iter!=_ussConnects.end()) {
        delete iter->second;
        _ussConnects.erase(iter++);
    }
}

//##ModelId=45750E810169
bool USSManService::start()
{
    MutexGuard guard(_mutex);
    if (_running)
        return true;

    if (!_server->Start()) {
        smsc_log_error(_logger, "%s: TCPSrv startup failure", _logId);
        return false;
    }
    if (!_disp->acRegistry()->getFactory(_ac_map_networkUnstructuredSs_v2)
        && !_disp->acRegistry()->regFactory(initMAPUSS2Components)) {
        smsc_log_fatal(_logger, "%s: ROS factory registration failed: %s!", _logId,
                      _ac_map_networkUnstructuredSs_v2.nick());
        return false;
    }
    if (!_disp->Start()) {
        smsc_log_error(_logger, "%s: TCAPDispatcher startup failure", _logId);
        return false;
    }
    if (!(_cfg.ssnSess = _disp->openSSN(_cfg.tcUsr.ownSsn, _cfg.tcUsr.maxDlgId, _logger))) {
        smsc_log_error(_logger, "%s: SSN[%u] initialization failure", _logId,
                       (unsigned)_cfg.tcUsr.ownSsn);
        return false;
    }
    smsc_log_debug(_logger, "%s: Started.", _logId);
    return (_running = true);
}

//##ModelId=45750EA80188
void USSManService::stop(bool do_wait/* = false*/)
{
    {
        MutexGuard guard(_mutex);
        if (!_running && !do_wait)
            return;
        if (_running) {
          ReverseMutexGuard rGrd(_mutex);
          //notify threads about stopping
          smsc_log_debug(_logger, "%s: Notifying TCP server ..", _logId);
          _server->Stop(0); //after that ConnectListenerITF methods are called
          smsc_log_debug(_logger, "%s: Notifyings TCAP dispatcher ..", _logId);
          _disp->Stop(false);
        }
        _running = false;
    }
    if (do_wait) { //wait for threads
        smsc_log_debug(_logger, "%s: Stopping TCP server ..", _logId);
        _server->Stop(); //after that ConnectListenerITF methods are called
        smsc_log_debug(_logger, "%s: Stopping TCAP dispatcher ..", _logId);
        _disp->Stop(true);
        smsc_log_debug(_logger, "%s: Stopped.", _logId);
    }
}

//##ModelId=45750E5702F0
smsc::inman::interaction::ConnectAC*
USSManService::onConnectOpening(smsc::inman::interaction::Server* srv,
                                    smsc::core::network::Socket* sock)
{
  std::auto_ptr<smsc::inman::interaction::Connect>
    conn(new smsc::inman::interaction::Connect(sock,
                    smsc::inman::interaction::SerializerUSS::getInstance(), _logger));

  smsc_log_debug(_logger, "%s::onConnectOpened: got new connection request on socket=%d",
                 _logId, sock->getSocket());
  USSManConnect *ussCon = new USSManConnect(conn->getId(), _logger, _cfg);
  if (ussCon) {
    smsc_log_info(_logger, "%s: New connect[%u] was created", _logId, sock->getSocket());
    smsc::core::synchronization::MutexGuard guard(_mutex);
    _ussConnects.insert(UssConnMap_t::value_type(sock->getSocket(), ussCon));
    conn->addListener(ussCon);
  }
  return conn.release();
}

//##ModelId=45750E68032E
void
USSManService::onConnectClosing(smsc::inman::interaction::Server* srv,
                                    smsc::inman::interaction::ConnectAC* conn)
{
  SOCKET sockId = conn->getSocket()->getSocket();
  smsc_log_debug(_logger, "%s::onConnectClosing: close connection on socket=%d", _logId, sockId);
  smsc::core::synchronization::MutexGuard guard(_mutex);

  UssConnMap_t::iterator it = _ussConnects.find(sockId);
  if (it != _ussConnects.end()) {
    USSManConnect *ussCon = it->second;
    (static_cast<smsc::inman::interaction::Connect*>(conn))->removeListener(ussCon);
    _ussConnects.erase(it);

    delete ussCon;
    smsc_log_info(_logger, "%s: Connect[%u] being closed", _logId, sockId);
  } else
    smsc_log_warn(_logger, "$s: attempt to close unknown connect[%u]", _logId, sockId);

}

//##ModelId=45750E720244
void
USSManService::onServerShutdown(smsc::inman::interaction::Server* srv,
                                    smsc::inman::interaction::Server::ShutdownReason reason)
{
  smsc_log_debug(_logger, "%s: TCP server shutdowning, reason %d", _logId, reason);
  srv->removeListener(this);
}

} //uss
} //inman
} //smsc

