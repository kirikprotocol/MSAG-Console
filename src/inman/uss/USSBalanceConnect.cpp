#include "USSBalanceConnect.hpp"
#include <logger/Logger.h>
#include "ussmessages.hpp"
#include "USSRequestProcessor.hpp"
#include <util/ObjectRegistry.hpp>

namespace smsc {
namespace inman {
namespace uss {

class USSProcSearchCrit {
public:
  USSProcSearchCrit(unsigned char ssn,
                    const TonNpiAddress& addr)
    : _ssn(ssn), _addr(addr) {}

  bool operator<(const USSProcSearchCrit& rhs) const {
    if ( _ssn < rhs._ssn ||
         _addr.toString() < rhs._addr.toString() ) return true;
    else return false;
  }
private:
  unsigned char _ssn;
  TonNpiAddress _addr;
};

USSBalanceConnect::USSBalanceConnect(smsc::logger::Logger* logger,
                                     const UssService_CFG& cfg)
  : _logger(logger), _cfg(cfg) {}

//##ModelId=4575350D008E
void USSBalanceConnect::onPacketReceived(smsc::inman::interaction::Connect* conn, // указатель на объект, обслуживающий соединение с smsc
                                         std::auto_ptr<smsc::inman::interaction::SerializablePacketAC>& recv_cmd) throw(std::exception)
{
  smsc_log_debug(_logger, "USSBalanceConnect::onCommandReceived::: Enter it");
  smsc::inman::interaction::USSPacketAC* requestPacket =
    static_cast<smsc::inman::interaction::USSPacketAC*>(recv_cmd.get());

  smsc::inman::interaction::USSRequestMessage* requestObject = 
    static_cast<smsc::inman::interaction::USSRequestMessage*>(requestPacket->pCmd());
  smsc_log_debug(_logger, "USSBalanceConnect::onCommandReceived::: got request object=[%s]",
                 requestObject->toString().c_str());

  USSRequestProcessor* ussReqProc = 
    ObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().getObject
    (
     USSProcSearchCrit(requestObject->get_IN_SSN(),
                       requestObject->get_IN_ISDNaddr())
     );

  if ( !ussReqProc ) {
    ussReqProc = new USSRequestProcessor(conn, _cfg);
    ObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().toRegisterObject
      (
       ussReqProc, USSProcSearchCrit(requestObject->get_IN_SSN(),
                                     requestObject->get_IN_ISDNaddr())
       );
  }

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
