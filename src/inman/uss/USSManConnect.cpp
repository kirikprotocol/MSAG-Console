#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <memory>
#include "util/ObjectRegistry.hpp"
#include "core/buffers/RefPtr.hpp"

#include "USSManConnect.hpp"
#include "ussmessages.hpp"
#include "USSRequestProcessor.hpp"

namespace smsc {
namespace inman {
namespace uss {

bool
DuplicateRequestChecker::isRequestRegistered(const USSProcSearchCrit& ussProcSearchCrit) const
{
  core::synchronization::MutexGuard mg(_lock);
  if ( _registeredRequests.find(ussProcSearchCrit) == _registeredRequests.end() ) return false;
  else return true;
}

void
DuplicateRequestChecker::registerRequest(const USSProcSearchCrit& ussProcSearchCrit)
{
  core::synchronization::MutexGuard mg(_lock);
  _registeredRequests.insert(ussProcSearchCrit);
}

void
DuplicateRequestChecker::unregisterRequest(const USSProcSearchCrit& ussProcSearchCrit)
{
  core::synchronization::MutexGuard mg(_lock);
  _registeredRequests.erase(ussProcSearchCrit);

}

USSManConnect::USSManConnect(unsigned conn_id, logger::Logger* logger,
                             const UssService_CFG & cfg)
  : _logger(logger), _cfg(cfg)
{
  snprintf(_logId, sizeof(_logId)-1, "Con[%u]", conn_id);
}

USSManConnect::~USSManConnect()
{
  core::synchronization::MutexGuard synchronize(_activeReqProcLock);
  while(!_activeReqProcessors.empty()) {
    active_req_processes_t::iterator iter = _activeReqProcessors.begin();
    (*iter)->markConnectAsClosed();
    _activeReqProcessors.erase(iter);
  }
}

void USSManConnect::onPacketReceived(interaction::Connect* conn, // указатель на объект, обслуживающий соединение с клиентом ussman'a
                                     std::auto_ptr<interaction::SerializablePacketAC>& recv_cmd)
{
  interaction::USSPacketAC* requestPacket =
    static_cast<interaction::USSPacketAC*>(recv_cmd.get());

  interaction::USSRequestMessage* requestObject =
    static_cast<interaction::USSRequestMessage*>(requestPacket->pCmd());
  smsc_log_info(_logger, "%s: received request=%s", _logId,
                 requestObject->toString().c_str());

  USSProcSearchCrit ussProcSearchCrit(requestObject->get_IN_SSN(),
                                      requestObject->get_IN_ISDNaddr(),
                                      requestPacket->dialogId(),
                                      conn);

  if ( DuplicateRequestChecker::getInstance().isRequestRegistered(ussProcSearchCrit) ) {
    smsc_log_error(_logger, "%s: received duplicate request=%s", _logId,
                   requestObject->toString().c_str());
    return;
  }
  DuplicateRequestChecker::getInstance().registerRequest(ussProcSearchCrit);
  std::auto_ptr<USSRequestProcessor> ussReqProc(
    new USSRequestProcessor(this, conn, _cfg,
                            requestPacket->dialogId(), ussProcSearchCrit, _logger)
    );
  {
    core::synchronization::MutexGuard synchronize(_activeReqProcLock);
    _activeReqProcessors.insert(ussReqProc.get());
  }
  ussReqProc->handleRequest(requestObject);
  ussReqProc.release();
}

void USSManConnect::onConnectError(interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
  smsc_log_debug(_logger, "%s: connect error %s", _logId,
                 p_exc.get() ? p_exc->what() : "unspecified");
}

void USSManConnect::markReqProcessorAsCompleted(USSRequestProcessor* ussReqProc)
{
  core::synchronization::MutexGuard synchronize(_activeReqProcLock);
  _activeReqProcessors.erase(ussReqProc);
}

}
}
}
