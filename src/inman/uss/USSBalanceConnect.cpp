#include "USSBalanceConnect.hpp"
#include <logger/Logger.h>
#include "ussmessages.hpp"
#include "USSRequestProcessor.hpp"
#include <util/ObjectRegistry.hpp>
#include <core/buffers/RefPtr.hpp>

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

USSBalanceConnect::USSBalanceConnect(smsc::logger::Logger* logger,
                                     const UssService_CFG& cfg)
  : _logger(logger), _cfg(cfg) {}

USSBalanceConnect::~USSBalanceConnect()
{
  //for (CreatedSearchCritList_t::iterator begin_iter=_searchCritForCreatedReqProcessors.begin(), end_iter=_searchCritForCreatedReqProcessors.end();
  //       begin_iter != end_iter; ++begin_iter) {
  //USSRequestProcessor* ussReqProc = 
  //    smsc::util::RefObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().toUnregisterObject(*begin_iter);
  //delete ussReqProc;
  //  }
}

//##ModelId=4575350D008E
void USSBalanceConnect::onPacketReceived(smsc::inman::interaction::Connect* conn, // указатель на объект, обслуживающий соединение с клиентом ussman'a
                                         std::auto_ptr<smsc::inman::interaction::SerializablePacketAC>& recv_cmd) throw(std::exception)
{
  smsc_log_debug(_logger, "USSBalanceConnect::onCommandReceived::: Enter it");
  smsc::inman::interaction::USSPacketAC* requestPacket =
    static_cast<smsc::inman::interaction::USSPacketAC*>(recv_cmd.get());

  smsc::inman::interaction::USSRequestMessage* requestObject = 
    static_cast<smsc::inman::interaction::USSRequestMessage*>(requestPacket->pCmd());
  smsc_log_debug(_logger, "USSBalanceConnect::onCommandReceived::: got request object=[%s]",
                 requestObject->toString().c_str());

  USSProcSearchCrit ussProcSearchCrit(requestObject->get_IN_SSN(),
                                      requestObject->get_IN_ISDNaddr(),
                                      requestPacket->dialogId(),
                                      conn);

  if ( DuplicateRequestChecker::getInstance().isRequestRegistered(ussProcSearchCrit) ) return;
  DuplicateRequestChecker::getInstance().registerRequest(ussProcSearchCrit);

  //smsc::core::buffers::RefPtr<USSRequestProcessor, smsc::core::synchronization::Mutex> ussReqProc =
  //smsc::util::RefObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().getObject(ussProcSearchCrit);

  USSRequestProcessor* ussReqProc = new USSRequestProcessor(conn, _cfg, ussProcSearchCrit);

  // if ( !ussReqProc.Get() ) {
  //     ussReqProc = smsc::core::buffers::RefPtr<USSRequestProcessor, smsc::core::synchronization::Mutex>(new USSRequestProcessor(conn, _cfg, ussProcSearchCrit));
  //     smsc::util::RefObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().toRegisterObject
  //       (
  //        ussReqProc,ussProcSearchCrit
  //        );

  //     _searchCritForCreatedReqProcessors.push_back(ussProcSearchCrit);
  //   }

  ussReqProc->setDialogId(requestPacket->dialogId());
  ussReqProc->handleRequest(requestObject);

  smsc_log_debug(_logger, "USSBalanceConnect::onCommandReceived::: Leave it");
}

//##ModelId=45753514006F
void USSBalanceConnect::onConnectError(smsc::inman::interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
  smsc_log_debug(_logger, "USSBalanceConnect::onConnectError::: Enter it");
  smsc_log_debug(_logger, "USSBalanceConnect::onConnectError::: Leave it");
}

}
}
}
