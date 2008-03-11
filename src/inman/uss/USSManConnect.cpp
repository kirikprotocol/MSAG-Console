#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

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
  smsc::core::synchronization::MutexGuard mg(_lock);
  if ( _registeredRequests.find(ussProcSearchCrit) == _registeredRequests.end() ) return false;
  else return true;
}

void
DuplicateRequestChecker::registerRequest(const USSProcSearchCrit& ussProcSearchCrit)
{
  smsc::core::synchronization::MutexGuard mg(_lock);
  _registeredRequests.insert(ussProcSearchCrit);
}

void
DuplicateRequestChecker::unregisterRequest(const USSProcSearchCrit& ussProcSearchCrit)
{
  smsc::core::synchronization::MutexGuard mg(_lock);
  _registeredRequests.erase(ussProcSearchCrit);

}

USSManConnect::USSManConnect(smsc::logger::Logger* logger,
                                     const UssService_CFG & cfg)
  : _logger(logger), _cfg(cfg)
{ }

USSManConnect::~USSManConnect()
{
  //for (CreatedSearchCritList_t::iterator begin_iter=_searchCritForCreatedReqProcessors.begin(), end_iter=_searchCritForCreatedReqProcessors.end();
  //       begin_iter != end_iter; ++begin_iter) {
  //USSRequestProcessor* ussReqProc = 
  //    smsc::util::RefObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().toUnregisterObject(*begin_iter);
  //delete ussReqProc;
  //  }
}

//##ModelId=4575350D008E
void USSManConnect::onPacketReceived(smsc::inman::interaction::Connect* conn, // указатель на объект, обслуживающий соединение с клиентом ussman'a
                                         std::auto_ptr<smsc::inman::interaction::SerializablePacketAC>& recv_cmd) throw(std::exception)
{
  smsc_log_debug(_logger, "USSManConnect::onPacketReceived: Enter it");
  smsc::inman::interaction::USSPacketAC* requestPacket =
    static_cast<smsc::inman::interaction::USSPacketAC*>(recv_cmd.get());

  smsc::inman::interaction::USSRequestMessage* requestObject = 
    static_cast<smsc::inman::interaction::USSRequestMessage*>(requestPacket->pCmd());
  smsc_log_debug(_logger, "USSManConnect::onPacketReceived: got request object=[%s]",
                 requestObject->toString().c_str());

  USSProcSearchCrit ussProcSearchCrit(requestObject->get_IN_SSN(),
                                      requestObject->get_IN_ISDNaddr(),
                                      requestPacket->dialogId(),
                                      conn);

  if ( DuplicateRequestChecker::getInstance().isRequestRegistered(ussProcSearchCrit) ) {
    smsc_log_error(_logger, "USSManConnect::onPacketReceived: got request object [%s] with duplicate reqId",
                   requestObject->toString().c_str());
    return;
  }
  DuplicateRequestChecker::getInstance().registerRequest(ussProcSearchCrit);
  USSRequestProcessor* ussReqProc = new USSRequestProcessor(conn, _cfg, ussProcSearchCrit, _logger);

  ussReqProc->setDialogId(requestPacket->dialogId());
  ussReqProc->handleRequest(requestObject);

  smsc_log_debug(_logger, "USSManConnect::onPacketReceived: Leave it");
}

//##ModelId=45753514006F
void USSManConnect::onConnectError(smsc::inman::interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
  smsc_log_debug(_logger, "USSManConnect::onConnectError: Enter it");
  smsc_log_debug(_logger, "USSManConnect::onConnectError: Leave it");
}

}
}
}
